/*
 * BSD 2-Clause License
 *
 * Purpose: Declarations for basic scheduling helpers.
 */
#ifndef SCHED_H
#define SCHED_H

int sched_yield(void);

/* scheduling priority interfaces */
#ifndef PRIO_PROCESS
#define PRIO_PROCESS 0
#endif
#ifndef PRIO_PGRP
#define PRIO_PGRP    1
#endif
#ifndef PRIO_USER
#define PRIO_USER    2
#endif
#ifndef PRIO_MIN
#define PRIO_MIN    -20
#endif
#ifndef PRIO_MAX
#define PRIO_MAX     20
#endif

int nice(int incr);
int getpriority(int which, int who);
int setpriority(int which, int who, int prio);

#endif /* SCHED_H */
