#ifndef FNMATCH_H
#define FNMATCH_H

/* pattern matching for filenames */

#define FNM_NOESCAPE 0x01
#define FNM_NOMATCH  1

int fnmatch(const char *pattern, const char *string, int flags);

#endif /* FNMATCH_H */
