/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the regex functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "regex.h"
#include "memory.h"
#include <string.h>
#include <ctype.h>

/*
 * This implementation is adapted from the public domain tiny-regex-c
 * project. It builds a very small NFA for the pattern and performs a
 * backtracking search. Only a subset of BRE/ERE syntax is supported:
 * '.', '*', '+', '?', '^', '$' and basic character classes.
 */

struct token {
    unsigned char type;
    union {
        unsigned char ch;
        unsigned char *ccl;
        int group;
    } u;
};

/* token types */
enum {
    UNUSED, DOT, BEGIN, END, QUESTION, STAR, PLUS, CHAR, CHAR_CLASS, INV_CHAR_CLASS,
    DIGIT, NOT_DIGIT, ALPHA, NOT_ALPHA, SPACE, NOT_SPACE,
    CAP_OPEN, CAP_CLOSE, BACKREF
};

struct regex_impl {
    struct token *pat;
    unsigned char *ccl;
};

/* helper prototypes */
static int matchpattern(struct token *pat, const char *text, int *matchlen,
                        const char *string, regmatch_t *caps);

static int matchdigit(char c) { return isdigit((unsigned char)c); }
static int matchalpha(char c) { return isalnum((unsigned char)c) || c == '_'; }
static int matchspace(char c) { return isspace((unsigned char)c); }

static int matchrange(char c, const char *str)
{
    return (c != '-') && str[0] && str[1] == '-' && str[2] &&
           c >= str[0] && c <= str[2];
}

static int matchdot(char c)
{
#ifdef RE_DOT_MATCHES_NEWLINE
    (void)c; return 1;
#else
    return c != '\n' && c != '\r';
#endif
}

static int ismeta(char c)
{
    return c=='s' || c=='S' || c=='w' || c=='W' || c=='d' || c=='D';
}

static int matchmeta(char c, const char *str)
{
    switch (str[0]) {
    case 'd': return  matchdigit(c);
    case 'D': return !matchdigit(c);
    case 'w': return  matchalpha(c);
    case 'W': return !matchalpha(c);
    case 's': return  matchspace(c);
    case 'S': return !matchspace(c);
    default:  return c == str[0];
    }
}

static int matchclass(char c, const char *str)
{
    do {
        if (matchrange(c, str))
            return 1;
        else if (str[0] == '\\') {
            str++;
            if (matchmeta(c, str))
                return 1;
            else if (c == str[0] && !ismeta(c))
                return 1;
        } else if (c == str[0]) {
            if (c == '-')
                return (str[-1] == '\0' || str[1] == '\0');
            return 1;
        }
    } while (*str++);
    return 0;
}

static int matchone(struct token t, char c)
{
    switch (t.type) {
    case DOT:            return matchdot(c);
    case CHAR_CLASS:     return  matchclass(c, (const char*)t.u.ccl);
    case INV_CHAR_CLASS: return !matchclass(c, (const char*)t.u.ccl);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalpha(c);
    case NOT_ALPHA:      return !matchalpha(c);
    case SPACE:          return  matchspace(c);
    case NOT_SPACE:      return !matchspace(c);
    default:             return t.u.ch == (unsigned char)c;
    }
}

static int matchstar(struct token p, struct token *pat, const char *text, int *ml,
                     const char *string, regmatch_t *caps)
{
    int pre = *ml;
    const char *save = text;
    while (*text && matchone(p, *text)) {
        text++; (*ml)++; }
    while (text >= save) {
        if (matchpattern(pat, text, ml, string, caps))
            return 1;
        text--; (*ml)--;
    }
    *ml = pre;
    return 0;
}

static int matchplus(struct token p, struct token *pat, const char *text, int *ml,
                     const char *string, regmatch_t *caps)
{
    const char *save = text;
    while (*text && matchone(p, *text)) { text++; (*ml)++; }
    while (text > save) {
        if (matchpattern(pat, text, ml, string, caps))
            return 1;
        text--; (*ml)--;
    }
    return 0;
}

static int matchquestion(struct token p, struct token *pat, const char *text, int *ml,
                         const char *string, regmatch_t *caps)
{
    if (p.type == UNUSED)
        return 1;
    if (matchpattern(pat, text, ml, string, caps))
        return 1;
    if (*text && matchone(p, *text++)) {
        if (matchpattern(pat, text, ml, string, caps)) {
            (*ml)++; return 1; }
    }
    return 0;
}

/* iterative matcher */
static int matchpattern(struct token *pat, const char *text, int *matchlen,
                        const char *string, regmatch_t *caps)
{
    int pre = *matchlen;
    do {
        if (pat[0].type == CAP_OPEN) {
            if (caps)
                caps[pat[0].u.group].rm_so = text - string;
            pat++;
            continue;
        } else if (pat[0].type == CAP_CLOSE) {
            if (caps)
                caps[pat[0].u.group].rm_eo = text - string;
            pat++;
            continue;
        } else if (pat[0].type == BACKREF) {
            int id = pat[0].u.group;
            if (!caps || caps[id].rm_so < 0 || caps[id].rm_eo < 0) {
                *matchlen = pre;
                return 0;
            }
            int blen = caps[id].rm_eo - caps[id].rm_so;
            if (strncmp(string + caps[id].rm_so, text, blen) != 0) {
                *matchlen = pre;
                return 0;
            }
            text += blen;
            *matchlen += blen;
            pat++;
            continue;
        }
        if (pat[0].type == UNUSED || pat[1].type == QUESTION)
            return matchquestion(pat[0], &pat[2], text, matchlen, string, caps);
        else if (pat[1].type == STAR)
            return matchstar(pat[0], &pat[2], text, matchlen, string, caps);
        else if (pat[1].type == PLUS)
            return matchplus(pat[0], &pat[2], text, matchlen, string, caps);
        else if (pat[0].type == END && pat[1].type == UNUSED)
            return *text == '\0';
        *matchlen += 1;
    } while (*text && matchone(*pat++, *text++));
    *matchlen = pre;
    return 0;
}

int regcomp(regex_t *preg, const char *pattern, int cflags)
{
    (void)cflags;
    size_t len = strlen(pattern);
    struct regex_impl *ri = calloc(1, sizeof(*ri));
    if (!ri)
        return -1;
    ri->pat = malloc((len + 1) * sizeof(struct token));
    ri->ccl = malloc(len * 2 + 1);
    if (!ri->pat || !ri->ccl) {
        free(ri->pat); free(ri->ccl); free(ri); return -1; }
    size_t ccl_idx = 1; /* first byte reserved */
    size_t i = 0, j = 0;
    int stack[10];
    int depth = 0;
    size_t groups = 0;
    while (pattern[i] && j + 1 < len) {
        char c = pattern[i];
        switch (c) {
        case '^': ri->pat[j].type = BEGIN; break;
        case '$': ri->pat[j].type = END;   break;
        case '.': ri->pat[j].type = DOT;   break;
        case '*': ri->pat[j].type = STAR;  break;
        case '+': ri->pat[j].type = PLUS;  break;
        case '?': ri->pat[j].type = QUESTION; break;
        case '\\':
            if (pattern[i+1]) {
                i++;
                if (pattern[i] >= '1' && pattern[i] <= '9') {
                    ri->pat[j].type = BACKREF;
                    ri->pat[j].u.group = pattern[i] - '0';
                } else switch (pattern[i]) {
                case 'd': ri->pat[j].type = DIGIT; break;
                case 'D': ri->pat[j].type = NOT_DIGIT; break;
                case 'w': ri->pat[j].type = ALPHA; break;
                case 'W': ri->pat[j].type = NOT_ALPHA; break;
                case 's': ri->pat[j].type = SPACE; break;
                case 'S': ri->pat[j].type = NOT_SPACE; break;
                default:  ri->pat[j].type = CHAR; ri->pat[j].u.ch = pattern[i]; break;
                }
            }
            break;
        case '(': {
            if (groups < 9) {
                groups++; depth++; stack[depth-1] = groups;
                ri->pat[j].type = CAP_OPEN;
                ri->pat[j].u.group = groups;
            } else {
                ri->pat[j].type = CHAR; ri->pat[j].u.ch = c;
            }
            break; }
        case ')': {
            if (depth > 0) {
                int id = stack[--depth];
                ri->pat[j].type = CAP_CLOSE;
                ri->pat[j].u.group = id;
            } else {
                ri->pat[j].type = CHAR; ri->pat[j].u.ch = c;
            }
            break; }
        case '[': {
            size_t start = ccl_idx;
            if (pattern[i+1] == '^') { ri->pat[j].type = INV_CHAR_CLASS; i++; }
            else ri->pat[j].type = CHAR_CLASS;
            while (pattern[++i] && pattern[i] != ']') {
                if (pattern[i] == '\\' && pattern[i+1]) {
                    ri->ccl[ccl_idx++] = pattern[i++];
                }
                ri->ccl[ccl_idx++] = pattern[i];
            }
            ri->ccl[ccl_idx++] = 0;
            ri->pat[j].u.ccl = &ri->ccl[start];
            break; }
        default:
            ri->pat[j].type = CHAR; ri->pat[j].u.ch = c; break;
        }
        i++; j++; if (ccl_idx + 1 >= len*2) break;
    }
    ri->pat[j].type = UNUSED;
    preg->impl = ri;
    preg->re_nsub = groups;
    return 0;
}

int regexec(const regex_t *preg, const char *string,
            size_t nmatch, regmatch_t pmatch[], int eflags)
{
    (void)eflags;
    if (!preg || !preg->impl)
        return REG_NOMATCH;
    struct regex_impl *ri = (struct regex_impl *)preg->impl;
    int len = 0;
    int pos = -1;
    regmatch_t *caps = NULL;
    if (preg->re_nsub) {
        caps = calloc(preg->re_nsub + 1, sizeof(regmatch_t));
        if (caps)
            memset(caps, -1, (preg->re_nsub + 1) * sizeof(regmatch_t));
    }
    if (ri->pat[0].type == BEGIN) {
        if (matchpattern(&ri->pat[1], string, &len, string, caps))
            pos = 0;
    } else {
        const char *t = string; int idx = -1;
        do {
            idx++; len = 0;
            if (caps)
                memset(caps, -1, (preg->re_nsub + 1) * sizeof(regmatch_t));
            if (matchpattern(ri->pat, t, &len, string, caps)) { pos = idx; break; }
        } while (*t++);
    }
    if (pos < 0)
    {
        free(caps);
        return REG_NOMATCH;
    }
    if (nmatch > 0) {
        pmatch[0].rm_so = pos;
        pmatch[0].rm_eo = pos + len;
        for (size_t i = 1; i <= preg->re_nsub && i < nmatch; i++)
            pmatch[i] = caps[i];
    }
    free(caps);
    return 0;
}

void regfree(regex_t *preg)
{
    if (!preg || !preg->impl)
        return;
    struct regex_impl *ri = (struct regex_impl *)preg->impl;
    free(ri->pat);
    free(ri->ccl);
    free(ri);
    preg->impl = NULL;
}

