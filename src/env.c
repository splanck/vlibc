/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the env functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "env.h"
#include "memory.h"
#include "string.h"

/* pointer to current environment */
char **environ = 0;
/* whether environ array was allocated by vlibc */
static int environ_owned = 0;
/* per-entry ownership flags when environ_owned */
static unsigned char *environ_flags = 0;

void env_init(char **envp)
{
    environ = envp;
    environ_owned = 0;
    environ_flags = 0;
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
        if (environ_owned)
            free(environ[idx]);
        environ[idx] = entry;
        return 0;
    }

    /* count items */
    int count = 0;
    if (environ) {
        while (environ[count])
            count++;
    }

    char **newenv;
    unsigned char *newflags;
    if (environ_owned) {
        newenv = realloc(environ, sizeof(char *) * (count + 2));
        newflags = realloc(environ_flags, sizeof(unsigned char) * (count + 2));
        if (!newenv || !newflags) {
            free(entry);
            return -1;
        }
        environ = newenv;
        environ_flags = newflags;
        for (int i = 0; i < count; ++i) {
            if (!environ_flags[i]) {
                char *dup = strdup(environ[i]);
                if (!dup) {
                    free(entry);
                    return -1;
                }
                environ[i] = dup;
                environ_flags[i] = 1;
            }
        }
    } else {
        newenv = malloc(sizeof(char *) * (count + 2));
        newflags = malloc(sizeof(unsigned char) * (count + 2));
        if (!newenv || !newflags) {
            free(entry);
            free(newenv);
            free(newflags);
            return -1;
        }
        for (int i = 0; i < count; ++i) {
            newenv[i] = strdup(environ[i]);
            if (!newenv[i]) {
                for (int j = 0; j < i; ++j)
                    free(newenv[j]);
                free(newenv);
                free(newflags);
                free(entry);
                return -1;
            }
            newflags[i] = 1;
        }
        environ_owned = 1;
        environ = newenv;
        environ_flags = newflags;
    }
    environ[count] = entry;
    if (environ_owned)
        environ_flags[count] = 1;
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

    char **newenv;
    unsigned char *newflags;
    if (environ_owned) {
        newenv = realloc(environ, sizeof(char *) * (count + 2));
        newflags = realloc(environ_flags, sizeof(unsigned char) * (count + 2));
        if (!newenv || !newflags)
            return -1;
        environ = newenv;
        environ_flags = newflags;
        for (int i = 0; i < count; ++i) {
            if (!environ_flags[i]) {
                char *dup = strdup(environ[i]);
                if (!dup)
                    return -1;
                environ[i] = dup;
                environ_flags[i] = 1;
            }
        }
    } else {
        newenv = malloc(sizeof(char *) * (count + 2));
        newflags = malloc(sizeof(unsigned char) * (count + 2));
        if (!newenv || !newflags) {
            free(newenv);
            free(newflags);
            return -1;
        }
        for (int i = 0; i < count; ++i) {
            newenv[i] = strdup(environ[i]);
            if (!newenv[i]) {
                for (int j = 0; j < i; ++j)
                    free(newenv[j]);
                free(newenv);
                free(newflags);
                return -1;
            }
            newflags[i] = 1;
        }
        environ_owned = 1;
        environ = newenv;
        environ_flags = newflags;
    }
    environ[count] = (char *)str;
    if (environ_owned)
        environ_flags[count] = 0;
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
    if (environ_owned && environ_flags && environ_flags[idx])
        free(environ[idx]);
    int i;
    for (i = idx; environ[i]; ++i) {
        environ[i] = environ[i + 1];
        if (environ_owned)
            environ_flags[i] = environ_flags[i + 1];
    }
    if (environ_owned)
        environ_flags[i] = 0;
    return 0;
}

int clearenv(void)
{
    if (!environ)
        return 0;
    if (!environ_owned) {
        int count = 0;
        while (environ[count])
            count++;
        char **newenv = malloc(sizeof(char *) * (count + 1));
        unsigned char *newflags = malloc(sizeof(unsigned char) * (count + 1));
        if (!newenv || !newflags) {
            free(newenv);
            free(newflags);
            return -1;
        }
        for (int i = 0; i < count; ++i) {
            newenv[i] = strdup(environ[i]);
            if (!newenv[i]) {
                for (int j = 0; j < i; ++j)
                    free(newenv[j]);
                free(newenv);
                free(newflags);
                return -1;
            }
            newflags[i] = 1;
        }
        newenv[count] = NULL;
        environ = newenv;
        environ_flags = newflags;
        environ_owned = 1;
    }
    for (int i = 0; environ[i]; ++i) {
        if (environ_flags && environ_flags[i])
            free(environ[i]);
    }
    free(environ);
    free(environ_flags);
    environ = malloc(sizeof(char *));
    environ_flags = malloc(sizeof(unsigned char));
    if (!environ || !environ_flags)
        return -1;
    environ_owned = 1;
    environ[0] = NULL;
    environ_flags[0] = 0;
    return 0;
}
