#ifndef INCLUDE_CORE_TESTING_PROFILES_H_
#define INCLUDE_CORE_TESTING_PROFILES_H_

#include "../utils/readconf.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <signal.h>
#include <sys/wait.h>

#define TESTING_PROFILE_NOT_TESTED 0
#define TESTING_PROFILE_SUCCESS 1
#define TESTING_PROFILE_FAIL 2
#define TESTING_PROFILE_RUNNING 3
#define TESTING_PROFILE_ERROR 4
#define TESTING_PROFILE_PARTIAL 5

#define NFQWS2_LOG_BUF_SIZE 16384

typedef struct {
  int profile_index;
  Profile profiles;
  char *zapret_path;
  int *results;
  pthread_mutex_t *mutex;
  Testing testing;
} TestingProfileJob;

typedef struct {
    int http_code;
    double latency_ms;
    int tcp_connected;
    int data_received;
    int success;
    char error_msg[256];
} test_result_t;

typedef struct {
    int test_success;
    int nfqws_exit_code;
    char logs[NFQWS2_LOG_BUF_SIZE];
    int desync_applied;
    int errors_detected;
    int queue_hit;
    int packet_received;
} full_test_result_t;

void testing_log_open(void);
void testing_log_close(void);
void testing_log(const char *func, const char *fmt, ...);

void* testing_profiles(void* arg);

int create_rule(TestingProfileJob *job, const char *tables, const int qnum);
char **build_nfqws2_args(int qnum, char *strategy, const char *zapret_path);
pid_t start_nfqws2(char *program, char **args, int *log_fd);
int stop_nfqws2_and_collect_logs(pid_t pid, int pipe_fd, char *logs_buf, size_t logs_buf_len);
full_test_result_t analyze_test(test_result_t http_res, int nfqws_exit, const char *logs);
const char* testing_profile_summary(full_test_result_t result);
test_result_t send_https_test_raw(const char *domain, int port, int mark, int timeout_sec);

int get_test_queue();
int add_iptables_nfqueue_rule(int queue_num);
int del_iptables_nfqueue_rule(int queue_num);

int add_nft_nfqueue_rule(int queue_num);
int del_nft_nfqueue_rule(int queue_num);

#endif  // INCLUDE_CORE_TESTING_PROFILES_H_
