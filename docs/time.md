[← Back to index](index.md)

## Time Formatting

`strftime` understands `%Y`, `%m`, `%d`, `%H`, `%M`, and `%S` along with `%a`, `%b`, `%Z`, `%z`, and weekday numbers (`%w`/`%u`).
`strptime` still parses the numeric fields (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`) back into a `struct tm` and returns a pointer to the first unparsed character or `NULL` on failure.

Example:

```c
struct tm tm = {
    .tm_year = 123, // 2023
    .tm_mon  = 4,   // May
    .tm_mday = 6,
    .tm_wday = 6,
    .tm_hour = 7,
    .tm_min  = 8,
    .tm_sec  = 9
};
char buf[64];
strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S %Z %z", &tm);
// "Sat May 06 2023 07:08:09 UTC +0000"
```
`wcsftime` performs the same conversion but writes to a wide-character buffer.
`timegm` converts a `struct tm` in UTC back to `time_t` using the same logic as `mktime` but without timezone adjustments.

### asctime and asctime_r

`asctime` converts a `struct tm` to the classic 26 byte string used by `ctime`.
`asctime_r` is the reentrant variant that writes into a caller provided buffer.

```c
time_t t = 1700000000;
struct tm tm;
gmtime_r(&t, &tm);
char buf[26];
printf("%s", asctime_r(&tm, buf));
// "Tue Nov 14 22:13:20 2023\n"
```

## Locale Support

`setlocale` reads `LC_ALL` and `LANG` and sets the active locale
accordingly. For non-`"C"` locales the function defers to the host
`setlocale(3)` implementation on BSD systems. `localeconv` returns
formatting information. `gmtime`, `localtime`, `mktime`, and `ctime`
convert between `time_t` and `struct tm` or human readable strings.

Locale objects provide per-thread settings. Use `newlocale` to create a
locale, `duplocale` to clone one, `freelocale` to free it and
`uselocale` to activate it. vlibc only implements the built-in `"C"`
locale on non-BSD systems while BSD builds wrap the native routines.

## Time Retrieval

Use `time`, `gettimeofday`, or `clock_gettime` to obtain the current time of day.

```c
time_t now = time(NULL);
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
```

The resolution of a clock can be determined with `clock_getres`:

```c
struct timespec res;
clock_getres(CLOCK_MONOTONIC, &res);
```

`difftime` subtracts two `time_t` values and returns the elapsed seconds as
a `double`.

Thread-safe variants `gmtime_r` and `localtime_r` fill a user-provided
`struct tm` using the same conversion logic.  `tzset` reloads the active
timezone from the `TZ` environment variable or `/etc/localtime` so
`localtime`, `mktime` and `ctime` honour the configured offset.

## Sleep Functions

Delay helpers are available in `time.h`:

```c
unsigned sleep(unsigned seconds);
int usleep(useconds_t usec);
int nanosleep(const struct timespec *req, struct timespec *rem);
unsigned int alarm(unsigned int seconds);
```

## Scheduling

`sched_yield` allows a thread to voluntarily relinquish the CPU, letting
other runnable threads run. When the `SYS_sched_yield` syscall is
available it is invoked directly. Otherwise the function falls back to
`nanosleep` with a zero timeout.

Declared in `<sched.h>`.

```c
int sched_yield(void);
```

`getpriority` returns the current scheduling priority for a process,
process group or user as selected by the `PRIO_*` constants.
`setpriority` updates the value and `nice` adjusts the calling
process' nice level relative to its current priority.  The wrappers
map directly to the `setpriority(2)` and `getpriority(2)` syscalls on
NetBSD and Linux, falling back to host implementations on the other
BSDs.

```c
int nice(int incr);
int getpriority(int which, int who);
int setpriority(int which, int who, int prio);
```

`sched_getscheduler` queries the current scheduling policy for a
process. `sched_setscheduler` changes the policy and priority using a
`struct sched_param`. `sched_getparam` and `sched_setparam` adjust only
the priority.  `sched_get_priority_max` and `sched_get_priority_min`
return the valid priority range for a policy and
`sched_rr_get_interval` reports the time slice for round-robin
scheduling.  When the corresponding syscalls are unavailable the
wrappers call the host implementations on BSD systems.

```c
int sched_getscheduler(pid_t pid);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_rr_get_interval(pid_t pid, struct timespec *interval);
```

## Interval Timers

`setitimer` schedules periodic `SIGALRM` delivery or CPU timers. `getitimer`
returns the remaining time and interval.

```c
struct itimerval it = { {1, 0}, {1, 0} };
setitimer(ITIMER_REAL, &it, NULL);
```

`timer_create` provides a more flexible POSIX timer API. After creating a
`timer_t` handle, call `timer_settime` to arm it and `timer_gettime` to query
the remaining time. vlibc maps these helpers to the Linux `timer_create(2)`
syscall. On NetBSD the native timer syscalls are preferred, while the other
BSDs use `kqueue` timers as a fallback when the syscalls are unavailable.

```c
timer_t t;
struct itimerspec its = { {0, 0}, {1, 0} };
timer_create(CLOCK_REALTIME, NULL, &t);
timer_settime(t, 0, &its, NULL);
```

## Resource Limits

Processes may query and update operating system limits using
`getrlimit` and `setrlimit` from `sys/resource.h`. See
[process.md](process.md) for process creation and signal helpers.

```c
struct rlimit lim;
if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
    printf("soft: %lu hard: %lu\n",
           (unsigned long)lim.rlim_cur,
           (unsigned long)lim.rlim_max);
}
```

`getrusage` reports detailed statistics about CPU time and other
resources consumed by a process. Call it with `RUSAGE_SELF` to inspect
the current process or `RUSAGE_CHILDREN` for exited children.

```c
struct rusage ru;
getrusage(RUSAGE_SELF, &ru);
printf("user: %ld.%06ld sys: %ld.%06ld\n",
       (long)ru.ru_utime.tv_sec, (long)ru.ru_utime.tv_usec,
       (long)ru.ru_stime.tv_sec, (long)ru.ru_stime.tv_usec);
```

