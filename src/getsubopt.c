/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the copyright notice and
 * this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements getsubopt for vlibc. Parses comma separated suboptions.
 */

#include "getopt.h"
#include "string.h"

int getsubopt(char **optionp, char * const *tokens, char **valuep)
{
    if (!optionp || !*optionp || !**optionp) {
        return -1;
    }

    char *arg = *optionp;
    char *end = arg;
    while (*end && *end != ',' && *end != '=')
        end++;

    char *val = NULL;
    if (*end == '=') {
        *end = '\0';
        val = end + 1;
        end = val;
        while (*end && *end != ',')
            end++;
        if (*end)
            *end = '\0';
    }

    if (*end == ',') {
        *end = '\0';
        *optionp = end + 1;
    } else {
        *optionp = end;
    }

    for (int i = 0; tokens && tokens[i]; i++) {
        if (strcmp(arg, tokens[i]) == 0) {
            *valuep = val;
            return i;
        }
    }

    *valuep = val;
    return -1;
}
