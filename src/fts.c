#include "fts.h"
#include "dirent.h"
#include "memory.h"
#include "string.h"
#include "errno.h"
#include "unistd.h"

struct node {
    struct node *next;
    char *path;
    int level;
};

struct _fts {
    struct node *head;
    struct node *tail;
    FTSENT *cur;
    int options;
    int (*compar)(const FTSENT **, const FTSENT **);
};

static int queue_push(FTS *fts, const char *path, int level)
{
    struct node *n = malloc(sizeof(*n));
    if (!n)
        return -1;
    n->path = strdup(path);
    if (!n->path) {
        free(n);
        return -1;
    }
    n->level = level;
    n->next = NULL;
    if (fts->tail)
        fts->tail->next = n;
    else
        fts->head = n;
    fts->tail = n;
    return 0;
}

static struct node *queue_pop(FTS *fts)
{
    struct node *n = fts->head;
    if (n) {
        fts->head = n->next;
        if (!fts->head)
            fts->tail = NULL;
    }
    return n;
}

FTS *fts_open(char * const *paths, int options,
              int (*compar)(const FTSENT **, const FTSENT **))
{
    if (!paths)
        return NULL;
    FTS *fts = calloc(1, sizeof(*fts));
    if (!fts)
        return NULL;
    fts->options = options;
    fts->compar = compar;
    for (size_t i = 0; paths[i]; i++) {
        if (queue_push(fts, paths[i], 0) < 0) {
            fts_close(fts);
            errno = ENOMEM;
            return NULL;
        }
    }
    return fts;
}

static void free_entry(FTSENT *e)
{
    if (!e)
        return;
    free(e->fts_path);
    free(e);
}

FTSENT *fts_read(FTS *fts)
{
    if (!fts)
        return NULL;
    free_entry(fts->cur);
    fts->cur = NULL;

    struct node *n = queue_pop(fts);
    if (!n)
        return NULL;

    struct stat st;
    int r;
    if (fts->options & FTS_PHYSICAL)
        r = lstat(n->path, &st);
    else
        r = stat(n->path, &st);

    FTSENT *ent = calloc(1, sizeof(*ent));
    if (!ent) {
        free(n->path);
        free(n);
        return NULL;
    }
    ent->fts_path = n->path;
    ent->fts_accpath = ent->fts_path;
    char *name = strrchr(ent->fts_path, '/');
    ent->fts_name = name ? name + 1 : ent->fts_path;
    ent->fts_namelen = strlen(ent->fts_name);
    ent->fts_level = n->level;
    if (r < 0) {
        ent->fts_info = FTS_NS;
    } else if (S_ISDIR(st.st_mode)) {
        ent->fts_info = FTS_D;
    } else if (S_ISLNK(st.st_mode)) {
        ent->fts_info = FTS_SL;
    } else {
        ent->fts_info = FTS_F;
    }
    if (r == 0)
        ent->fts_stat = st;

    if (ent->fts_info == FTS_D) {
        DIR *d = opendir(ent->fts_path);
        if (!d) {
            ent->fts_info = FTS_DNR;
        } else {
            struct dirent *e;
            while ((e = readdir(d))) {
                if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
                    continue;
                size_t len = strlen(ent->fts_path);
                size_t add = len && ent->fts_path[len-1] != '/' ? 1 : 0;
                char *child = malloc(len + add + strlen(e->d_name) + 1);
                if (!child) {
                    closedir(d);
                    free_entry(ent);
                    free(n);
                    return NULL;
                }
                memcpy(child, ent->fts_path, len);
                if (add)
                    child[len] = '/';
                strcpy(child + len + add, e->d_name);
                queue_push(fts, child, n->level + 1);
                free(child);
            }
            closedir(d);
        }
    }

    free(n);
    fts->cur = ent;
    return ent;
}

int fts_close(FTS *fts)
{
    if (!fts)
        return -1;
    free_entry(fts->cur);
    struct node *n;
    while ((n = queue_pop(fts))) {
        free(n->path);
        free(n);
    }
    free(fts);
    return 0;
}

