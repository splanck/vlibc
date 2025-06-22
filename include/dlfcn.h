#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY   1
#define RTLD_NOW    2
#define RTLD_LOCAL  0
#define RTLD_GLOBAL 0x100

void *dlopen(const char *filename, int flag);
void *dlsym(void *handle, const char *symbol);
int dlclose(void *handle);
const char *dlerror(void);

#endif /* DLFCN_H */
