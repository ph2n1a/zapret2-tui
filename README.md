# zapret2-tui

> TUI wrapper for [zapret2](https://github.com/bol-van/zapret2) — DPI bypass system for Linux

**[Русская версия](README_ru.md)**

<div align="center">
  <p align="center"><img src="program.png" width="600" alt="screenshot" /></p>
</div>

## What is it

zapret2-tui is a text user interface (TUI) for managing [zapret2](https://github.com/bol-van/zapret2). This program is not a standalone version of zapret2, but an add-on.

The core idea: all DPI bypass strategies are split into **profiles** for quick switching. Each profile is its own strategy.

## How it works

```
┌──────────────────────────────────────────────────────────────────┐
│                         zapret2-tui                              │
│                                                                  │
│  ┌─────────────────────┐     ┌──────────────────────────────┐    │
│  │   App's config      │     │      zapret2's config        │    │
│  │   buffer            │     │      /opt/zapret2/config     │    │
│  │                     │     │                              │    │
│  │  ┌───────────────┐  │     │  ┌─────────────────────────┐ │    │
│  │  │ Profile 1     │  │     │  │  ACTIVE config          │ │    │
│  │  │ (strategy A)  │──┼────>│  │  (copied from profile   │ │    │
│  │  ├───────────────┤  │ cp  │  └─────────────────────────┘ │    │
│  │  │ Profile 2     │  │     │                              │    │
│  │  │ (strategy B)  │  │     └──────────────────────────────┘    │
│  │  ├───────────────┤  │                                         │
│  │  │ Profile 3     │  │     ┌──────────────────────────────┐    │
│  │  │ (strategy C)  │  │     │   systemctl start/stop       │    │
│  │  └───────────────┘  │     │   zapret2.service            │    │
│  └─────────────────────┘     └──────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────┘
```

### How it works

1. **Profile storage**: The app stores its own copies of the zapret2 config file with different strategies. Each profile is a complete copy of the config, but with different NFQWS2_OPT values.

2. **Profile switching**: When you select a profile, the app simply **copies** the stored config over zapret2's main config file (`/opt/zapret2/config`). The selected profile is highlighted with **square brackets** `[ ]` in the interface.

3. **Service management**: The app can manage the zapret2 service via systemctl:
   - `S` — start the service
   - `X` — stop the service
   - `R` — restart the service

4. **Profile testing** (optional): The app can test profiles by:
   - Creating an iptables/nftables rule for NFQUEUE (queue 2001)
   - Starting nfqws2 with the profile's strategy
   - Sending an HTTPS request through the queue
   - Checking if the strategy was applied and the site is accessible

## Dependencies

- **[zapret2](https://github.com/bol-van/zapret2)** — core system (required)
- **Linux** with NFQUEUE support (kernel module `nfnetlink_queue`)
- **ncurses** — for TUI interface
- **OpenSSL** — for TLS testing
- **libconfuse** — for config parsing (built automatically)
- **pthreads** — for multi-threaded testing
- **root** — required for iptables/nftables and NFQUEUE management


### Automatic dependency building

During build, these are automatically downloaded and compiled:
- libconfuse (via FetchContent)
- ncurses (via ExternalProject)

## First run

```bash
sudo ./build/zapret2-tui
```

On first run, the program will ask for:
1. Path to the zapret2 folder (e.g. `/opt/zapret2`)
2. Choice: continue or edit `profiles.conf`

## File structure

```
zapret2-tui/
├── config/
│   └── config              # Main config (auto-generated)
├── profiles.conf           # Strategy profiles (created manually)
├── logs/
│   ├── zapret2-tui.log     # Application logs
│   └── testing.log         # Detailed testing logs
└── zapret2-tui             # Binary
```

## Controls

### Hotkeys

| Key | Action |
|-----|--------|
| `↑` / `↓` | Select profile |
| `Enter` | Apply selected profile |
| `S` | Start zapret2 |
| `X` | Stop zapret2 |
| `R` | Restart zapret2 |
| `T` | Test current profile |
| `A` | Test all profiles |
| `H` | Help |
| `Q` / `Esc` | Quit |

### Profile statuses

| Letter | Color | Meaning |
|--------|-------|---------|
| **S** | 🟢 green | SUCCESS — strategy works |
| **P** | 🔵 cyan | PARTIAL — site accessible, but no desync detected |
| **F** | 🔴 red | FAIL — strategy doesn't work |
| **T** | 🟡 yellow | TESTING — test in progress |
| **X** | 🟣 magenta | ERROR — nfqws2 error |

## Profiles (profiles.conf)

```
testing {
  tables = "iptables"
  domain_test = "rutracker.org"
}

profile {
  id = 0
  name = "my profile"
  NFQWS2_OPT = "--filter-tcp=443 --filter-l7=tls --payload=tls_client_hello \
  --lua-desync=fake:blob=fake_default_tls:tcp_md5:tcp_seq=-10000 \
  --lua-desync=multidisorder:pos=1,midsld"
}
```

### NFQWS2_OPT formatting rules

- Line break: backslash `\` at end of line
- Variable names: `<HOSTLIST>`, `<HOSTLIST_NOAUTO>`
- Strategies separated by `--new`
- Available lua functions: `fake`, `multisplit`, `multidisorder`, `wssize`, `pktmod`, `send`

### Strategy examples

**Basic (fake packet):**
```
--filter-tcp=443 --filter-l7=tls --payload=tls_client_hello \
--lua-desync=fake:blob=fake_default_tls:tcp_md5:tcp_seq=-10000
```

**Multisplit:**
```
--filter-tcp=443 --filter-l7=tls --payload=tls_client_hello \
--lua-desync=multisplit:pos=1,sniext+1,host+1,midsld
```

**Combined:**
```
--filter-tcp=443 --filter-l7=tls --payload=tls_client_hello \
--lua-desync=fake:blob=fake_default_tls:tcp_md5:repeats=1 \
--lua-desync=multisplit:pos=1,sniext+1,host+1,midsld-2,midsld,midsld+2,endhost-1 \
--payload=empty --out-range=<s1 --lua-desync=send:tcp_md5
```

## Profile testing

The program can test profiles by sending HTTPS requests through NFQUEUE:

1. An iptables/nftables rule is created for queue 2001
2. nfqws2 starts with the specified profile
3. An HTTPS request is sent to the test domain
4. The result is analyzed: whether the strategy was applied, whether the site is accessible

### Test results

| Status | Meaning |
|--------|---------|
| **SUCCESS** | Strategy applied, site accessible |
| **PARTIAL** | Site accessible, but no desync detected (possibly DPI doesn't block) |
| **FAIL** | Strategy broke the connection or doesn't help |

## Logging

All actions are logged to two files:

| File | Purpose |
|------|---------|
| `./logs/zapret2-tui.log` | General application logs |
| `./logs/testing.log` | Detailed testing logs |

Log format:
```
[2026-06-19 09:20:19] [module] [LEVEL]: message
```

Levels: `INFO`, `WARNING`, `ERROR`

## Architecture

```
src/
├── main.c                    # Entry point, chdir to program directory
├── core/
│   ├── testing_profiles.c    # Profile testing logic
│   ├── create_zapret_configs.c  # zapret config generation
│   ├── create_link.c         # Profile application
│   ├── read_zapret_conf.c    # zapret config reading
│   └── first_start.c         # First-run wizard
├── ui/
│   ├── app.c                 # Main application loop
│   ├── core.c                # Action handling logic
│   ├── ui.c                  # Interface rendering
│   └── input.c               # Input handling
└── utils/
    ├── log.c                 # Logging system
    ├── readconf.c            # Config reading
    ├── writeconf.c           # Config writing
    └── utils.c               # Utilities
```



- [zapret2](https://github.com/bol-van/zapret2) — core DPI bypass system
- [libconfuse](https://github.com/martinh/libconfuse) — config parsing library
- ncurses — TUI interface library
