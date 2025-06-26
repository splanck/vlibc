[← Back to index](index.md)

## System-V IPC

vlibc exposes thin wrappers around the classical System‑V IPC
facilities.  When running on BSD the native C library functions are
invoked directly, while on Linux the wrappers issue the corresponding
syscalls.

### Example

```c
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main(void)
{
    int shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0600);
    void *mem = shmat(shmid, NULL, 0);

    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
#ifdef _SEM_SEMUN_UNDEFINED
    union semun { int val; struct semid_ds *buf; unsigned short *array; } arg;
#else
    union semun arg;
#endif
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1); /* lock */
    semop(semid, &(struct sembuf){0, 1, 0}, 1); /* unlock */

    shmdt(mem);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
```
