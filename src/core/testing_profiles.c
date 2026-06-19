#include "../../include/core/testing_profiles.h"
#include "../../include/utils/log.h"
#include <stdio.h>

void testing_log_open(void) {
  log_open();
}

void testing_log_close(void) {
  log_close();
}

void testing_log(const char *func, const char *fmt, ...) {
  char buf[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  log_msg(func, LOG_INFO, "%s", buf);
  test_log_msg(func, LOG_INFO, "%s", buf);
}


static bool wait_for_nfqws2_ready(int log_fd, int timeout_ms) {
  char buf[4096];
  ssize_t total = 0;
  struct pollfd pfd = {.fd = log_fd, .events = POLLIN};
  struct timespec start, now;
  clock_gettime(CLOCK_MONOTONIC, &start);

  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &now);
    long elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;
    if (elapsed_ms >= timeout_ms) {
      testing_log("wait_nfqws2", "WARN: timeout waiting for nfqws2 ready (%dms)", timeout_ms);
      break;
    }

    int ret = poll(&pfd, 1, 100);
    if (ret < 0) {
      if (errno == EINTR) continue;
      break;
    }

    if (pfd.revents & POLLIN) {
      ssize_t n = read(log_fd, buf + total, sizeof(buf) - 1 - total);
      if (n <= 0) break;
      total += n;
      buf[total] = '\0';

      if (strstr(buf, "binding this socket to queue") != NULL) {
        testing_log("wait_nfqws2", "INFO: nfqws2 ready (found binding message)");
        return true;
      }
    }

    if (pfd.revents & (POLLHUP | POLLERR)) {
      break;
    }
  }

  return false;
}

static bool nfqws2_exit_is_expected(int exit_code) {
  return exit_code == 0 || exit_code == 1 || exit_code == 128 + SIGTERM;
}

static int testing_profile_verdict(full_test_result_t result) {
  if (!nfqws2_exit_is_expected(result.nfqws_exit_code)) {
    testing_log("verdict", "ERROR: unexpected exit code %d", result.nfqws_exit_code);
    return TESTING_PROFILE_ERROR;
  }

  if (strstr(result.logs, "execv failed") != NULL) {
    testing_log("verdict", "ERROR: execv failed detected in logs");
    return TESTING_PROFILE_ERROR;
  }

  if (strstr(result.logs, "Need queue number") != NULL) {
    testing_log("verdict", "ERROR: nfqws2 didn't receive --qnum argument");
    return TESTING_PROFILE_ERROR;
  }

  if (!result.queue_hit && !result.desync_applied) {
    testing_log("verdict", "ERROR: no packets received AND no desync — nfqws2 not working");
    return TESTING_PROFILE_ERROR;
  }

  if (result.test_success && result.desync_applied && !result.errors_detected) {
    testing_log("verdict", "SUCCESS: http_code=2xx desync=1 errors=0");
    return TESTING_PROFILE_SUCCESS;
  }

  if (result.test_success && !result.desync_applied && !result.errors_detected) {
    testing_log("verdict", "PARTIAL: http_code=2xx but no desync detected — site works without strategy");
    return TESTING_PROFILE_PARTIAL;
  }

  if (!result.test_success && result.desync_applied) {
    testing_log("verdict", "FAIL: desync applied but http failed — strategy breaks connection");
    return TESTING_PROFILE_FAIL;
  }

  if (!result.test_success && !result.desync_applied) {
    testing_log("verdict", "FAIL: http failed and no desync — strategy not working");
    return TESTING_PROFILE_FAIL;
  }

  testing_log("verdict", "FAIL: test_success=%d desync=%d errors=%d exit_code=%d",
              result.test_success, result.desync_applied, result.errors_detected, result.nfqws_exit_code);
  return TESTING_PROFILE_FAIL;
}

void testing_profile_set_result(TestingProfileJob *job, int result) {
  pthread_mutex_lock(job->mutex);
  job->results[job->profile_index] = result;
  pthread_mutex_unlock(job->mutex);
}

int delete_rule(const char *tables, const int qnum) {
  if (!tables) {
    testing_log("delete_rule", "ERROR: tables is NULL");
    return -1;
  }
  if (strcmp(tables, "iptables") == 0) {
    return del_iptables_nfqueue_rule(qnum);
  }
  if (strcmp(tables, "nftables") == 0) {
    return del_nft_nfqueue_rule(qnum);
  }
  testing_log("delete_rule", "ERROR: unknown table type '%s'", tables);
  return -1;
}

static int delete_rule_with_retry(const char *tables, int qnum, int retries) {
  for (int i = 0; i < retries; i++) {
    if (delete_rule(tables, qnum) == 0) {
      return 0;
    }
    testing_log("delete_rule_with_retry", "WARN: delete_rule attempt %d/%d failed for qnum %d",
                i + 1, retries, qnum);
    if (i < retries - 1) {
      usleep(200000);
    }
  }
  return -1;
}

void *testing_profiles(void *arg) {
  TestingProfileJob *job = arg;
  if (!job || !job->zapret_path || !job->results || !job->mutex) {
    testing_log("testing_profiles", "ERROR: invalid job parameters");
    return NULL;
  }
  if (job->testing.error_code) {
    testing_log("testing_profiles", "ERROR: testing.error_code is set");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }
  if (!is_root()) {
    testing_log("testing_profiles", "ERROR: not running as root");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }

  Profile profiles = job->profiles;
  char *zapret_path = job->zapret_path;
  const char *tables = job->testing.tables;
  const char *domain = job->testing.domain;

  if (!domain || domain[0] == '\0') {
    testing_log("testing_profiles", "ERROR: domain is NULL or empty");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }
  if (!tables || tables[0] == '\0') {
    testing_log("testing_profiles", "ERROR: tables is NULL or empty");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }

  testing_log("testing_profiles", "INFO: starting test profile '%s' domain='%s' tables='%s'",
              profiles.name, domain, tables);

  int qnum = -1;
  pid_t pid = -1;
  char program[512];
  char nfqws2_logs[NFQWS2_LOG_BUF_SIZE];
  int log_fd = -1;
  int final_result = TESTING_PROFILE_ERROR;
  bool rule_created = false;

  qnum = get_test_queue();
  if (qnum == -1) {
    testing_log("testing_profiles", "ERROR: no free nf_queue available");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }
  testing_log("testing_profiles", "INFO: using nf_queue %d", qnum);

  if (create_rule(job, tables, qnum) == -1) {
    testing_log("testing_profiles", "ERROR: create_rule failed for tables='%s' qnum=%d", tables, qnum);
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return NULL;
  }
  rule_created = true;

  char **args = build_nfqws2_args(qnum, profiles.nfqws2_opt, zapret_path);
  if (!args) {
    testing_log("testing_profiles", "ERROR: build_nfqws2_args returned NULL");
    final_result = TESTING_PROFILE_ERROR;
    goto cleanup;
  }

  if (snprintf(program, sizeof(program), "%s/nfq2/nfqws2", zapret_path) >= (int)sizeof(program)) {
    testing_log("testing_profiles", "ERROR: program path truncated");
    final_result = TESTING_PROFILE_ERROR;
    goto cleanup;
  }

  if (access(program, X_OK) != 0) {
    testing_log("testing_profiles", "ERROR: nfqws2 binary not found or not executable: %s", program);
    final_result = TESTING_PROFILE_ERROR;
    goto cleanup;
  }

  pid = start_nfqws2(program, args, &log_fd);
  if (pid <= 0) {
    testing_log("testing_profiles", "ERROR: start_nfqws2 failed (pid=%d)", pid);
    final_result = TESTING_PROFILE_ERROR;
    goto cleanup;
  }
  testing_log("testing_profiles", "INFO: nfqws2 started pid=%d", pid);

  bool binding_confirmed = wait_for_nfqws2_ready(log_fd, 5000);

  test_result_t http_res = send_https_test_raw(domain, 443, 99, 5);

  const int nfqws2_code = stop_nfqws2_and_collect_logs(pid, log_fd, nfqws2_logs, sizeof(nfqws2_logs));
  close(log_fd);
  log_fd = -1;

  testing_log("testing_profiles", "INFO: nfqws2 exit_code=%d http_code=%d success=%d",
              nfqws2_code, http_res.http_code, http_res.success);
  if (nfqws2_logs[0] != '\0') {
    testing_log("testing_profiles", "NFQWS2_OUTPUT: %.4096s", nfqws2_logs);
  }

  full_test_result_t result = analyze_test(http_res, nfqws2_code, nfqws2_logs);
  if (binding_confirmed && result.packet_received) {
    result.queue_hit = 1;
  }
  final_result = testing_profile_verdict(result);

  const char *summary = testing_profile_summary(result);
  testing_log("testing_profiles", "SUMMARY: %s", summary);

cleanup:
  if (log_fd >= 0) {
    close(log_fd);
  }

  if (rule_created) {
    if (delete_rule_with_retry(tables, qnum, 3) == -1) {
      testing_log("testing_profiles", "CRITICAL: failed to delete rule after 3 retries! qnum=%d tables='%s'", qnum, tables);
      final_result = TESTING_PROFILE_ERROR;
    }
  }

  testing_log("testing_profiles", "INFO: test completed with result=%d", final_result);
  testing_profile_set_result(job, final_result);

  return NULL;
}

int create_rule(TestingProfileJob *job, const char *tables, const int qnum) {
  if (!tables) {
    testing_log("create_rule", "ERROR: tables is NULL");
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return -1;
  }
  if (strcmp(tables, "iptables") == 0) {
    if (add_iptables_nfqueue_rule(qnum) == -1) {
      testing_log("create_rule", "ERROR: add_iptables_nfqueue_rule failed qnum=%d", qnum);
      testing_profile_set_result(job, TESTING_PROFILE_ERROR);
      return -1;
    }
  } else if (strcmp(tables, "nftables") == 0) {
    if (add_nft_nfqueue_rule(qnum) == -1) {
      testing_log("create_rule", "ERROR: add_nft_nfqueue_rule failed qnum=%d", qnum);
      testing_profile_set_result(job, TESTING_PROFILE_ERROR);
      return -1;
    }
  } else {
    testing_log("create_rule", "ERROR: unknown table type '%s'", tables);
    testing_profile_set_result(job, TESTING_PROFILE_ERROR);
    return -1;
  }

  return 0;
}

char **build_nfqws2_args(int qnum, char *strategy, const char *zapret_path) {
  static char nfqws2_args[128][1024];
  static char *argv[128];
  memset(nfqws2_args, 0, sizeof(nfqws2_args));

  strcpy(nfqws2_args[0], "nfqws2");
  snprintf(nfqws2_args[1], 1024, "--qnum=%d", qnum);
  strcpy(nfqws2_args[2], "--debug=1");

  if (zapret_path && zapret_path[0] != '\0') {
    snprintf(nfqws2_args[3], 1024, "--lua-init=@%s/lua/zapret-lib.lua", zapret_path);
    snprintf(nfqws2_args[4], 1024, "--lua-init=@%s/lua/zapret-antidpi.lua", zapret_path);
    snprintf(nfqws2_args[5], 1024, "--lua-init=@%s/lua/zapret-auto.lua", zapret_path);
  } else {
    testing_log("build_nfqws2_args", "WARN: zapret_path is NULL/empty, using hardcoded paths");
    strcpy(nfqws2_args[3], "--lua-init=@/opt/zapret2/lua/zapret-lib.lua");
    strcpy(nfqws2_args[4], "--lua-init=@/opt/zapret2/lua/zapret-antidpi.lua");
    strcpy(nfqws2_args[5], "--lua-init=@/opt/zapret2/lua/zapret-auto.lua");
  }

  int idx = 6;
  if (strategy && strategy[0] != '\0') {
    char *str_copy = strdup(strategy);
    if (str_copy) {
      char *token = strtok(str_copy, " ");
      while (token != NULL && idx < 127) {
        strncpy(nfqws2_args[idx], token, 1023);
        nfqws2_args[idx][1023] = '\0';
        idx++;
        token = strtok(NULL, " ");
      }
      free(str_copy);
    } else {
      testing_log("build_nfqws2_args", "ERROR: strdup failed for strategy");
    }
  }

  for (int i = 0; i < idx; i++) {
    argv[i] = nfqws2_args[i];
  }
  argv[idx] = NULL;

  testing_log("build_nfqws2_args", "INFO: argc=%d argv[0]='%s' argv[1]='%s'", idx, argv[0], argv[1]);

  return argv;
}

pid_t start_nfqws2(char *program, char **args, int *log_fd) {
  int pfd[2];
  if (pipe(pfd) == -1) {
    testing_log("start_nfqws2", "ERROR: pipe() failed: %s", strerror(errno));
    return -1;
  }

  if (args) {
    char cmdline[2048] = {0};
    int pos = 0;
    for (int i = 0; args[i] && pos < (int)sizeof(cmdline) - 1; i++) {
      if (i > 0) cmdline[pos++] = ' ';
      int n = snprintf(cmdline + pos, sizeof(cmdline) - pos, "%s", args[i]);
      if (n > 0) pos += n;
    }
    testing_log("start_nfqws2", "INFO: exec: %s %s", program, cmdline);
  }

  pid_t pid = fork();
  if (pid < 0) {
    testing_log("start_nfqws2", "ERROR: fork() failed: %s", strerror(errno));
    close(pfd[0]);
    close(pfd[1]);
    return -1;
  }

  if (pid == 0) {
    close(pfd[0]);

    if (dup2(pfd[1], STDOUT_FILENO) < 0) {
      perror("dup2 stdout");
      _exit(EXIT_FAILURE);
    }
    if (dup2(pfd[1], STDERR_FILENO) < 0) {
      perror("dup2 stderr");
      _exit(EXIT_FAILURE);
    }
    close(pfd[1]);

    execv(program, args);

    perror("execv failed");
    _exit(EXIT_FAILURE);
  }

  close(pfd[1]);

  if (log_fd) {
    *log_fd = pfd[0];
  } else {
    close(pfd[0]);
  }

  return pid;
}

int stop_nfqws2_and_collect_logs(pid_t pid, int pipe_fd, char *logs_buf, size_t logs_buf_len) {
  if (pid <= 0) {
    testing_log("stop_nfqws2", "ERROR: invalid pid %d", pid);
    return -1;
  }

  kill(pid, SIGTERM);
  testing_log("stop_nfqws2", "INFO: sent SIGTERM to pid %d", pid);

  struct pollfd pfd = {.fd = pipe_fd, .events = POLLIN};
  int status = -1;
  ssize_t total_read = 0;
  bool process_exited = false;

  struct timespec loop_start, loop_now;
  clock_gettime(CLOCK_MONOTONIC, &loop_start);

  while (total_read < (ssize_t)(logs_buf_len - 1)) {
    clock_gettime(CLOCK_MONOTONIC, &loop_now);
    long elapsed_ms = (loop_now.tv_sec - loop_start.tv_sec) * 1000 + (loop_now.tv_nsec - loop_start.tv_nsec) / 1000000;
    if (elapsed_ms > 5000) {
      testing_log("stop_nfqws2", "WARN: drain timeout after %ldms", elapsed_ms);
      break;
    }

    int ret = poll(&pfd, 1, 500);
    if (ret < 0) {
      if (errno == EINTR) continue;
      testing_log("stop_nfqws2", "ERROR: poll() failed: %s", strerror(errno));
      break;
    }

    if (pfd.revents & POLLIN) {
      ssize_t n = read(pipe_fd, logs_buf + total_read, logs_buf_len - 1 - total_read);
      if (n < 0) {
        if (errno == EINTR) continue;
        testing_log("stop_nfqws2", "ERROR: read() failed: %s", strerror(errno));
        break;
      }
      if (n == 0) break;
      total_read += n;
    }

    if (pfd.revents & (POLLHUP | POLLERR)) {
      break;
    }

    pid_t w = waitpid(pid, &status, WNOHANG);
    if (w == pid) {
      process_exited = true;
      break;
    }
    if (w < 0 && errno != EINTR) {
      testing_log("stop_nfqws2", "WARN: waitpid() returned error: %s", strerror(errno));
      break;
    }
  }

  logs_buf[total_read] = '\0';

  if (!process_exited) {
    struct timespec kill_start, kill_now;
    clock_gettime(CLOCK_MONOTONIC, &kill_start);
    while (1) {
      pid_t w = waitpid(pid, &status, WNOHANG);
      if (w == pid) {
        process_exited = true;
        break;
      }
      if (w < 0 && errno != EINTR) {
        break;
      }
      clock_gettime(CLOCK_MONOTONIC, &kill_now);
      long elapsed_ms = (kill_now.tv_sec - kill_start.tv_sec) * 1000 + (kill_now.tv_nsec - kill_start.tv_nsec) / 1000000;
      if (elapsed_ms > 2000) {
        break;
      }
      usleep(50000);
    }

    if (!process_exited) {
      kill(pid, SIGKILL);
      testing_log("stop_nfqws2", "WARN: sent SIGKILL to pid %d", pid);
      if (waitpid(pid, &status, 0) < 0) {
        testing_log("stop_nfqws2", "ERROR: waitpid after SIGKILL failed: %s", strerror(errno));
        return -1;
      }
    }
  }

  if (WIFEXITED(status)) {
    int code = WEXITSTATUS(status);
    testing_log("stop_nfqws2", "INFO: process exited normally with code %d", code);
    return code;
  }
  if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);
    testing_log("stop_nfqws2", "INFO: process killed by signal %d (%s)", sig,
                sig == SIGTERM ? "SIGTERM" : sig == SIGKILL ? "SIGKILL" : "unknown");
    return 128 + sig;
  }
  testing_log("stop_nfqws2", "WARN: unknown exit status %d", status);
  return -1;
}

full_test_result_t analyze_test(test_result_t http_res, int nfqws_exit, const char *logs) {
  full_test_result_t res = {0};

  res.test_success = http_res.success;
  res.nfqws_exit_code = nfqws_exit;

  if (!logs) {
    testing_log("analyze_test", "WARN: logs is NULL");
    return res;
  }

  bool queue_bound = (strstr(logs, "binding this socket to queue") != NULL);
  res.packet_received = (strstr(logs, "packet:") != NULL);
  res.queue_hit = queue_bound && res.packet_received;

  res.desync_applied = (strstr(logs, "* lua '") && strstr(logs, ": desync") != NULL) || (strstr(logs, "LUA: fake:") != NULL || strstr(logs, "LUA: multisplit:") != NULL || strstr(logs, "LUA: wssize:") != NULL || strstr(logs, "LUA: pktmod:") != NULL);
  res.errors_detected = (strstr(logs, "[ERROR]") != NULL || strstr(logs, "execv failed") != NULL);

  testing_log("analyze_test", "INFO: queue_bound=%d packet_received=%d desync=%d errors=%d http_success=%d http_code=%d",
              queue_bound, res.packet_received, res.desync_applied, res.errors_detected, http_res.success, http_res.http_code);

  strncpy(res.logs, logs, sizeof(res.logs) - 1);
  res.logs[sizeof(res.logs) - 1] = '\0';

  return res;
}

const char* testing_profile_summary(full_test_result_t result) {
  if (!nfqws2_exit_is_expected(result.nfqws_exit_code)) {
    return "ERROR: nfqws2 crashed";
  }
  if (strstr(result.logs, "execv failed")) {
    return "ERROR: execv failed";
  }
  if (strstr(result.logs, "Need queue number")) {
    return "ERROR: nfqws2 missing --qnum";
  }
  if (!result.queue_hit && !result.desync_applied) {
    return "ERROR: nfqws2 not working";
  }
  if (result.test_success && result.desync_applied && !result.errors_detected) {
    return "OK: strategy works, site accessible";
  }
  if (result.test_success && !result.desync_applied && !result.errors_detected) {
    return "PARTIAL: site works but no desync detected";
  }
  if (!result.test_success && result.desync_applied) {
    return "FAIL: desync applied but site not accessible";
  }
  if (!result.test_success && !result.desync_applied) {
    return "FAIL: site not accessible, no desync";
  }
  if (result.errors_detected) {
    return "FAIL: errors in nfqws2 logs";
  }
  return "FAIL: unknown reason";
}

int socket_with_mark(int mark) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    testing_log("socket_with_mark", "ERROR: socket() failed: %s", strerror(errno));
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_MARK, &mark, sizeof(mark)) < 0) {
    testing_log("socket_with_mark", "ERROR: setsockopt(SO_MARK) failed: %s", strerror(errno));
    close(sock);
    return -1;
  }

  int buf_size = 65536;
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
  setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));

  return sock;
}

int resolve_domain(const char *domain, struct addrinfo **res_out) {
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  int ret = getaddrinfo(domain, NULL, &hints, res_out);
  if (ret != 0) {
    testing_log("resolve_domain", "ERROR: getaddrinfo('%s') failed: %s", domain, gai_strerror(ret));
    return -1;
  }
  return 0;
}

int connect_with_timeout(int sock, const struct sockaddr *addr, socklen_t addrlen, int timeout_sec) {
  int flags = fcntl(sock, F_GETFL, 0);
  if (flags < 0) {
    testing_log("connect_with_timeout", "ERROR: fcntl(F_GETFL) failed: %s", strerror(errno));
    return -1;
  }

  if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
    testing_log("connect_with_timeout", "ERROR: fcntl(F_SETFL) failed: %s", strerror(errno));
    return -1;
  }

  int ret = connect(sock, addr, addrlen);

  if (ret < 0) {
    if (errno == EINPROGRESS) {
      struct pollfd pfd = {.fd = sock, .events = POLLOUT};
      int poll_ret = poll(&pfd, 1, timeout_sec * 1000);

      if (poll_ret <= 0) {
        fcntl(sock, F_SETFL, flags);
        return -1;
      }

      int err = 0;
      socklen_t len = sizeof(err);
      if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        fcntl(sock, F_SETFL, flags);
        return -1;
      }
      if (err != 0) {
        errno = err;
        fcntl(sock, F_SETFL, flags);
        return -1;
      }
    } else {
      fcntl(sock, F_SETFL, flags);
      return -1;
    }
  }

  fcntl(sock, F_SETFL, flags);
  return 0;
}

test_result_t send_https_test_raw(const char *domain, int port, int mark, int timeout_sec) {
  test_result_t res = {0};
  res.http_code = 0;
  res.success = 0;
  int sock = -1;
  int use_tls = (port == 443);
  SSL_CTX *ssl_ctx = NULL;
  SSL *ssl = NULL;
  struct addrinfo *addr_res = NULL;
  struct timespec t_start, t_connect, t_first_byte;

  if (!domain || domain[0] == '\0') {
    snprintf(res.error_msg, sizeof(res.error_msg), "domain is NULL or empty");
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    return res;
  }

  if (resolve_domain(domain, &addr_res) != 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "DNS resolution failed for %s", domain);
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    return res;
  }

  sock = socket_with_mark(mark);
  if (sock < 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "Failed to create socket");
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }

  struct sockaddr_in *addr_in = (struct sockaddr_in *)addr_res->ai_addr;
  addr_in->sin_port = htons(port);

  clock_gettime(CLOCK_MONOTONIC, &t_start);

  if (connect_with_timeout(sock, (struct sockaddr *)addr_in, sizeof(*addr_in), timeout_sec) != 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "TCP connect failed: %s", strerror(errno));
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }
  res.tcp_connected = 1;

  clock_gettime(CLOCK_MONOTONIC, &t_connect);
  res.latency_ms = (t_connect.tv_sec - t_start.tv_sec) * 1000.0 + (t_connect.tv_nsec - t_start.tv_nsec) / 1000000.0;

  struct timeval io_timeout = {.tv_sec = timeout_sec, .tv_usec = 0};
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &io_timeout, sizeof(io_timeout));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &io_timeout, sizeof(io_timeout));

  char request[512];
  int req_len = snprintf(request, sizeof(request),
                         "GET / HTTP/1.1\r\n"
                         "Host: %s\r\n"
                         "User-Agent: zapret2-tui-tester/1.0\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         domain);

  if (req_len < 0 || req_len >= (int)sizeof(request)) {
    snprintf(res.error_msg, sizeof(res.error_msg), "HTTP request is too large");
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }

  if (use_tls) {
    ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!ssl_ctx) {
      unsigned long ssl_err = ERR_get_error();
      ERR_error_string_n(ssl_err, res.error_msg, sizeof(res.error_msg));
      testing_log("send_https_test_raw", "ERROR: SSL_CTX_new failed: %s", res.error_msg);
      goto cleanup;
    }

    ssl = SSL_new(ssl_ctx);
    if (!ssl) {
      unsigned long ssl_err = ERR_get_error();
      ERR_error_string_n(ssl_err, res.error_msg, sizeof(res.error_msg));
      testing_log("send_https_test_raw", "ERROR: SSL_new failed: %s", res.error_msg);
      goto cleanup;
    }

    if (SSL_set_tlsext_host_name(ssl, domain) != 1 ||
        SSL_set_fd(ssl, sock) != 1) {
      unsigned long ssl_err = ERR_get_error();
      ERR_error_string_n(ssl_err, res.error_msg, sizeof(res.error_msg));
      testing_log("send_https_test_raw", "ERROR: SSL setup failed: %s", res.error_msg);
      goto cleanup;
    }

    int ssl_ret = SSL_connect(ssl);
    if (ssl_ret != 1) {
      int ssl_reason = SSL_get_error(ssl, ssl_ret);
      unsigned long ssl_err = ERR_get_error();
      char ssl_msg[128] = "unknown TLS error";
      if (ssl_err) {
        ERR_error_string_n(ssl_err, ssl_msg, sizeof(ssl_msg));
      }
      snprintf(res.error_msg, sizeof(res.error_msg), "TLS connect failed (%d): %s", ssl_reason, ssl_msg);
      testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
      goto cleanup;
    }

    ssl_ret = SSL_write(ssl, request, req_len);
    if (ssl_ret <= 0) {
      int ssl_reason = SSL_get_error(ssl, ssl_ret);
      unsigned long ssl_err = ERR_get_error();
      char ssl_msg[128] = "unknown TLS error";
      if (ssl_err) {
        ERR_error_string_n(ssl_err, ssl_msg, sizeof(ssl_msg));
      }
      snprintf(res.error_msg, sizeof(res.error_msg), "TLS write failed (%d): %s", ssl_reason, ssl_msg);
      testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
      goto cleanup;
    }
  } else if (send(sock, request, req_len, 0) < 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "send() failed: %s", strerror(errno));
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }

  char buf[4096];
  struct pollfd pfd = {.fd = sock, .events = POLLIN};

  int poll_ret = poll(&pfd, 1, timeout_sec * 1000);
  if (poll_ret <= 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "recv() timeout");
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }

  clock_gettime(CLOCK_MONOTONIC, &t_first_byte);

  ssize_t n;
  if (use_tls) {
    int ssl_ret = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (ssl_ret <= 0) {
      int ssl_reason = SSL_get_error(ssl, ssl_ret);
      unsigned long ssl_err = ERR_get_error();
      char ssl_msg[128] = "unknown TLS error";
      if (ssl_err) {
        ERR_error_string_n(ssl_err, ssl_msg, sizeof(ssl_msg));
      }
      snprintf(res.error_msg, sizeof(res.error_msg), "TLS read failed (%d): %s", ssl_reason, ssl_msg);
      testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
      goto cleanup;
    }
    n = ssl_ret;
  } else {
    n = recv(sock, buf, sizeof(buf) - 1, 0);
  }
  if (n <= 0) {
    snprintf(res.error_msg, sizeof(res.error_msg), "recv() failed or empty");
    testing_log("send_https_test_raw", "ERROR: %s", res.error_msg);
    goto cleanup;
  }

  buf[n] = '\0';
  res.data_received = 1;

  res.latency_ms = (t_first_byte.tv_sec - t_start.tv_sec) * 1000.0 + (t_first_byte.tv_nsec - t_start.tv_nsec) / 1000000.0;

  if (sscanf(buf, "HTTP/%*f %d", &res.http_code) != 1) {
    res.http_code = 0;
  }

  res.success = (res.http_code >= 200 && res.http_code < 400);
  testing_log("send_https_test_raw", "INFO: domain=%s http_code=%d latency=%.1fms",
              domain, res.http_code, res.latency_ms);

cleanup:
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  if (ssl_ctx)
    SSL_CTX_free(ssl_ctx);
  if (sock >= 0)
    close(sock);
  if (addr_res)
    freeaddrinfo(addr_res);
  return res;
}

int get_test_queue() {
  char cmd[512];

  // Clean any stale iptables rules on queue 2001
  snprintf(cmd, sizeof(cmd),
           "iptables -S OUTPUT 2>/dev/null | grep 'NFQUEUE --queue-num 2001' | sed 's/-A/-D/' | sh 2>/dev/null; true");
  system(cmd);

  // Clean any stale nft rules
  snprintf(cmd, sizeof(cmd),
           "nft delete rule inet filter output tcp dport 443 queue num 2001 2>/dev/null; true");
  system(cmd);

  testing_log("get_test_queue", "Using fixed queue 2001, stale rules cleaned");
  return 2001;
}

int add_iptables_nfqueue_rule(int queue_num) {
  if (queue_num < 2001 || queue_num > 2100) {
    testing_log("add_iptables", "ERROR: queue number %d out of range", queue_num);
    return -1;
  }

  char cmd[256];
  int ret = snprintf(cmd, sizeof(cmd),
                     "iptables -A OUTPUT -p tcp --dport 443 -m mark --mark 99 -j "
                     "NFQUEUE --queue-num %d",
                     queue_num);
  if (ret < 0 || ret >= (int)sizeof(cmd)) {
    testing_log("add_iptables", "ERROR: command truncated");
    return -1;
  }

  ret = system(cmd);
  if (ret != 0) {
    testing_log("add_iptables", "ERROR: iptables -A failed (return code: %d) cmd='%s'", ret, cmd);
    return -1;
  }

  testing_log("add_iptables", "INFO: rule added qnum=%d", queue_num);
  return 0;
}

int del_iptables_nfqueue_rule(int queue_num) {
  if (queue_num < 2001 || queue_num > 2100) {
    testing_log("del_iptables", "ERROR: queue number %d out of range", queue_num);
    return -1;
  }

  char cmd[256];
  int ret = snprintf(cmd, sizeof(cmd),
                     "iptables -D OUTPUT -p tcp --dport 443 -m mark --mark 99 -j "
                     "NFQUEUE --queue-num %d",
                     queue_num);
  if (ret < 0 || ret >= (int)sizeof(cmd)) {
    testing_log("del_iptables", "ERROR: command truncated");
    return -1;
  }

  ret = system(cmd);
  if (ret != 0) {
    testing_log("del_iptables", "ERROR: iptables -D failed (return code: %d) cmd='%s'", ret, cmd);
    return -1;
  }

  testing_log("del_iptables", "INFO: rule deleted qnum=%d", queue_num);
  return 0;
}

int add_nft_nfqueue_rule(int queue_num) {
  if (queue_num < 2001 || queue_num > 2100) {
    testing_log("add_nft", "ERROR: queue number %d out of range", queue_num);
    return -1;
  }
  if (!is_root()) {
    testing_log("add_nft", "ERROR: root privileges required");
    return -1;
  }
  char cmd[512];
  snprintf(cmd, sizeof(cmd), "nft add table inet filter");
  if (system(cmd) != 0) {
    testing_log("add_nft", "WARN: nft add table may already exist");
  }
  snprintf(cmd, sizeof(cmd), "nft add chain inet filter output type filter hook output priority 0");
  if (system(cmd) != 0) {
    testing_log("add_nft", "WARN: nft add chain may already exist");
  }
  snprintf(cmd, sizeof(cmd), "nft add rule inet filter output tcp dport 443 meta mark 99 queue num %d", queue_num);
  int ret = system(cmd);
  if (ret != 0) {
    testing_log("add_nft", "ERROR: nft add rule failed (return code: %d)", ret);
    return -1;
  }

  testing_log("add_nft", "INFO: rule added qnum=%d", queue_num);
  return 0;
}

int del_nft_nfqueue_rule(int queue_num) {
  if (queue_num < 2001 || queue_num > 2100) {
    testing_log("del_nft", "ERROR: queue number %d out of range", queue_num);
    return -1;
  }
  if (!is_root()) {
    testing_log("del_nft", "ERROR: root privileges required");
    return -1;
  }
  char cmd[512];
  snprintf(cmd, sizeof(cmd),
           "nft delete rule inet filter output tcp dport 443 meta mark 99 "
           "queue num %d",
           queue_num);
  int ret = system(cmd);
  if (ret != 0) {
    testing_log("del_nft", "ERROR: nft delete rule failed (return code: %d)", ret);
    return -1;
  }

  testing_log("del_nft", "INFO: rule deleted qnum=%d", queue_num);
  return 0;
}
