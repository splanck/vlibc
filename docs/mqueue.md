[← Back to index](index.md)

## POSIX Message Queues

vlibc wraps the standard POSIX message queue interface. Queues are opened with
`mq_open` and cleaned up via `mq_close` and `mq_unlink`. Basic send and receive
operations are provided along with functions to query attributes and perform
operations with timeouts.

### Attributes

`mq_getattr` fetches the current settings for a queue such as `mq_flags`,
`mq_maxmsg`, `mq_msgsize` and the number of pending messages. `mq_setattr`
updates the flags (typically `O_NONBLOCK`) and can return the previous values.

### Timed Send and Receive

`mq_timedsend` and `mq_timedreceive` behave like `mq_send` and `mq_receive` but
accept an absolute timeout. If the operation cannot complete before the timeout
expires they fail with `ETIMEDOUT`.

```c
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
ts.tv_sec += 1; /* wait up to one second */
mq_timedsend(mq, "hello", 6, 0, &ts);

char buf[16];
ts.tv_sec += 1;
mq_timedreceive(mq, buf, sizeof(buf), NULL, &ts);
```

