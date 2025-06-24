#include "wordexp.h"
#include "glob.h"
#include "memory.h"
#include "string.h"
#include "env.h"
#include <ctype.h>
#include <stdlib.h>

struct part {
    char *text;
    int do_glob;
};

static int add_word(wordexp_t *we, const char *str)
{
    char **tmp = realloc(we->we_wordv, sizeof(char*) * (we->we_wordc + 2));
    if (!tmp)
        return -1;
    we->we_wordv = tmp;
    we->we_wordv[we->we_wordc] = strdup(str);
    if (!we->we_wordv[we->we_wordc])
        return -1;
    we->we_wordc++;
    we->we_wordv[we->we_wordc] = NULL;
    return 0;
}

static int do_glob_expand(wordexp_t *we, struct part *p)
{
    if (!p->do_glob) {
        return add_word(we, p->text);
    }
    glob_t g;
    int r = glob(p->text, 0, NULL, &g);
    if (r == 0) {
        for (size_t i = 0; i < g.gl_pathc; i++) {
            if (add_word(we, g.gl_pathv[i]) < 0) {
                globfree(&g);
                return -1;
            }
        }
        globfree(&g);
        return 0;
    }
    if (r == GLOB_NOMATCH) {
        return add_word(we, p->text);
    }
    return -1;
}

static int parse_word(const char **strp, struct part *out)
{
    const char *s = *strp;
    size_t cap = 16;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf)
        return -1;
    int glob = 1;
    int q = 0; /* 1 single, 2 double */
    while (*s) {
        char c = *s;
        if (!q && isspace((unsigned char)c))
            break;
        if (!q && c == '\\') {
            s++;
            if (*s) {
                if (len + 1 >= cap) {
                    cap *= 2; char *nb = realloc(buf, cap); if (!nb) { free(buf); return -1; }
                    buf = nb;
                }
                buf[len++] = *s;
                if (*s=='*' || *s=='?' || *s=='[') glob=0; /* escaped */
                s++; continue;
            }
            break;
        }
        if (!q && c == '\'') {
            q = 1; s++; continue;
        }
        if (q==1) {
            if (c == '\'') { q=0; s++; continue; }
            if (len + 1 >= cap) { cap*=2; char *nb = realloc(buf, cap); if(!nb){ free(buf); return -1;} buf=nb; }
            buf[len++] = c; s++; continue;
        }
        if (!q && c == '"') { q=2; s++; continue; }
        if (q==2) {
            if (c == '"') { q=0; s++; continue; }
            if (c == '\\' && s[1]) {
                c = s[1]; s += 2;
            } else {
                s++;
            }
            if (len + 1 >= cap) { cap*=2; char *nb = realloc(buf, cap); if(!nb){ free(buf); return -1;} buf=nb; }
            buf[len++] = c;
            continue;
        }
        /* unquoted char */
        if (c=='~' && len==0) {
            const char *home = getenv("HOME");
            if (!home) home = "";
            size_t hlen = strlen(home);
            if (len + hlen >= cap) { while(len + hlen >= cap) cap*=2; char *nb=realloc(buf, cap); if(!nb){free(buf);return -1;} buf=nb; }
            memcpy(buf+len, home, hlen); len += hlen; s++; continue;
        }
        if (c=='*' || c=='?' || c=='[') glob = 1;
        if (len + 1 >= cap) { cap*=2; char *nb=realloc(buf, cap); if(!nb){free(buf);return -1;} buf=nb; }
        buf[len++] = c;
        s++;
    }
    buf[len]='\0';
    out->text = buf;
    out->do_glob = glob;
    *strp = s;
    return 0;
}

int wordexp(const char *words, wordexp_t *pwordexp)
{
    if (!words || !pwordexp)
        return WRDE_BADCHAR;
    pwordexp->we_wordc = 0;
    pwordexp->we_wordv = NULL;
    pwordexp->we_offs = 0;
    const char *p = words;
    struct part part;
    while (1) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        if (parse_word(&p, &part) < 0) { wordfree(pwordexp); return WRDE_NOSPACE; }
        if (do_glob_expand(pwordexp, &part) < 0) { free(part.text); wordfree(pwordexp); return WRDE_NOSPACE; }
        free(part.text);
    }
    return 0;
}

void wordfree(wordexp_t *pwordexp)
{
    if (!pwordexp) return;
    for (size_t i=0;i<pwordexp->we_wordc;i++) free(pwordexp->we_wordv[i]);
    free(pwordexp->we_wordv);
    pwordexp->we_wordv = NULL;
    pwordexp->we_wordc = 0;
    pwordexp->we_offs = 0;
}

