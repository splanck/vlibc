#ifndef ENV_H
#define ENV_H

/* Access to environment variables */

/* global environment pointer */
extern char **environ;

/* initialize environment from envp */
void env_init(char **envp);

char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

#endif /* ENV_H */
