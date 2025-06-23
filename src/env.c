#include "env.h"
#include "memory.h"
#include "string.h"

/* pointer to current environment */
char **environ = 0;

void env_init(char **envp)
{
    environ = envp;
}

char *getenv(const char *name)
{
    if (!environ || !name)
        return NULL;
    size_t len = strlen(name);
    for (char **e = environ; *e; ++e) {
        if (strncmp(*e, name, len) == 0 && (*e)[len] == '=')
            return *e + len + 1;
    }
    return NULL;
}

static int find_env_index(const char *name, size_t len)
{
    if (!environ)
        return -1;
    for (int i = 0; environ[i]; ++i) {
        if (strncmp(environ[i], name, len) == 0 && environ[i][len] == '=')
            return i;
    }
    return -1;
}

int setenv(const char *name, const char *value, int overwrite)
{
    if (!name || strchr(name, '='))
        return -1;
    if (!value)
        value = "";
    size_t nlen = strlen(name);
    size_t vlen = strlen(value);

    int idx = find_env_index(name, nlen);
    if (idx >= 0 && !overwrite)
        return 0;

    char *entry = malloc(nlen + vlen + 2); /* name=value\0 */
    if (!entry)
        return -1;
    vmemcpy(entry, name, nlen);
    entry[nlen] = '=';
    vmemcpy(entry + nlen + 1, value, vlen);
    entry[nlen + 1 + vlen] = '\0';

    if (idx >= 0) {
        environ[idx] = entry;
        return 0;
    }

    /* count items */
    int count = 0;
    if (environ) {
        while (environ[count])
            count++;
    }

    char **newenv = realloc(environ, sizeof(char *) * (count + 2));
    if (!newenv) {
        return -1;
    }
    environ = newenv;
    environ[count] = entry;
    environ[count + 1] = NULL;
    return 0;
}

int putenv(const char *str)
{
    if (!str)
        return -1;
    const char *eq = strchr(str, '=');
    if (!eq)
        return -1;

    size_t nlen = (size_t)(eq - str);

    int idx = find_env_index(str, nlen);
    if (idx >= 0) {
        environ[idx] = (char *)str;
        return 0;
    }

    int count = 0;
    if (environ) {
        while (environ[count])
            count++;
    }

    char **newenv = realloc(environ, sizeof(char *) * (count + 2));
    if (!newenv)
        return -1;
    environ = newenv;
    environ[count] = (char *)str;
    environ[count + 1] = NULL;
    return 0;
}

int unsetenv(const char *name)
{
    if (!environ || !name || strchr(name, '='))
        return -1;
    size_t len = strlen(name);
    int idx = find_env_index(name, len);
    if (idx < 0)
        return 0;
    for (int i = idx; environ[i]; ++i)
        environ[i] = environ[i + 1];
    return 0;
}
