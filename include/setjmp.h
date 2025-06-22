#ifndef SETJMP_H
#define SETJMP_H

/* simple jump buffer for x86_64 */
typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val) __attribute__((noreturn));

#endif /* SETJMP_H */
