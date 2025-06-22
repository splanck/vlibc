#include "dlfcn.h"
#include "memory.h"
#include "string.h"
#include "io.h"
#include "errno.h"
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

struct dl_handle {
    void *mapping;
    size_t map_size;
    void *base;
    Elf64_Sym *symtab;
    const char *strtab;
    size_t nsyms;
};

static char dl_err[128];

static ssize_t pread_fd(int fd, void *buf, size_t count, off_t offset)
{
    if (lseek(fd, offset, SEEK_SET) < 0)
        return -1;
    return read(fd, buf, count);
}

static void set_error(const char *msg)
{
    size_t len = strlen(msg);
    if (len >= sizeof(dl_err))
        len = sizeof(dl_err) - 1;
    memcpy(dl_err, msg, len);
    dl_err[len] = '\0';
}

const char *dlerror(void)
{
    if (dl_err[0] == '\0')
        return NULL;
    return dl_err;
}

void *dlopen(const char *filename, int flag)
{
    (void)flag;
    dl_err[0] = '\0';

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        set_error("open failed");
        return NULL;
    }

    Elf64_Ehdr eh;
    if (read(fd, &eh, sizeof(eh)) != sizeof(eh)) {
        set_error("read header");
        close(fd);
        return NULL;
    }
    if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
        eh.e_ident[EI_MAG1] != ELFMAG1 ||
        eh.e_ident[EI_MAG2] != ELFMAG2 ||
        eh.e_ident[EI_MAG3] != ELFMAG3 ||
        eh.e_ident[EI_CLASS] != ELFCLASS64 ||
        eh.e_type != ET_DYN) {
        set_error("not a shared object");
        close(fd);
        return NULL;
    }

    if (lseek(fd, eh.e_phoff, SEEK_SET) < 0) {
        set_error("seek phdr");
        close(fd);
        return NULL;
    }

    Elf64_Phdr phdrs[16];
    if (eh.e_phnum > 16) {
        set_error("too many phdrs");
        close(fd);
        return NULL;
    }

    if (read(fd, phdrs, eh.e_phnum * sizeof(Elf64_Phdr)) != (ssize_t)(eh.e_phnum * sizeof(Elf64_Phdr))) {
        set_error("read phdrs");
        close(fd);
        return NULL;
    }

    Elf64_Addr min_vaddr = ~0ULL;
    Elf64_Addr max_vaddr = 0;
    Elf64_Addr dyn_vaddr = 0;
    for (int i = 0; i < eh.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            if (phdrs[i].p_vaddr < min_vaddr)
                min_vaddr = phdrs[i].p_vaddr;
            if (phdrs[i].p_vaddr + phdrs[i].p_memsz > max_vaddr)
                max_vaddr = phdrs[i].p_vaddr + phdrs[i].p_memsz;
        } else if (phdrs[i].p_type == PT_DYNAMIC) {
            dyn_vaddr = phdrs[i].p_vaddr;
        }
    }

    size_t map_size = max_vaddr - min_vaddr;
    void *map = mmap(NULL, map_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANON, -1, 0);
    if (map == MAP_FAILED) {
        set_error("mmap");
        close(fd);
        return NULL;
    }

    void *base = (char *)map - min_vaddr;
    for (int i = 0; i < eh.e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD)
            continue;
        int prot = 0;
        if (phdrs[i].p_flags & PF_R) prot |= PROT_READ;
        if (phdrs[i].p_flags & PF_W) prot |= PROT_WRITE;
        if (phdrs[i].p_flags & PF_X) prot |= PROT_EXEC;
        void *seg_addr = (char *)base + phdrs[i].p_vaddr;
        if (pread_fd(fd, seg_addr, phdrs[i].p_filesz, phdrs[i].p_offset) != (ssize_t)phdrs[i].p_filesz) {
            munmap(map, map_size);
            set_error("read segment");
            close(fd);
            return NULL;
        }
        if (phdrs[i].p_memsz > phdrs[i].p_filesz)
            memset((char *)seg_addr + phdrs[i].p_filesz, 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
        mprotect(seg_addr, phdrs[i].p_memsz, prot);
    }

    struct dl_handle *h = malloc(sizeof(struct dl_handle));
    if (!h) {
        munmap(map, map_size);
        set_error("nomem");
        close(fd);
        return NULL;
    }
    h->mapping = map;
    h->map_size = map_size;
    h->base = base;
    h->symtab = NULL;
    h->strtab = NULL;
    h->nsyms = 0;

    if (dyn_vaddr) {
        Elf64_Dyn *dyn = (Elf64_Dyn *)((char *)base + dyn_vaddr);
        Elf64_Rela *rela = NULL;
        size_t relasz = 0;
        size_t syment = sizeof(Elf64_Sym);
        for (; dyn->d_tag != DT_NULL; dyn++) {
            if (dyn->d_tag == DT_SYMTAB)
                h->symtab = (Elf64_Sym *)((char *)base + dyn->d_un.d_ptr);
            else if (dyn->d_tag == DT_STRTAB)
                h->strtab = (const char *)((char *)base + dyn->d_un.d_ptr);
            else if (dyn->d_tag == DT_HASH) {
                uint32_t *hash = (uint32_t *)((char *)base + dyn->d_un.d_ptr);
                h->nsyms = hash[1];
            } else if (dyn->d_tag == DT_SYMENT) {
                syment = dyn->d_un.d_val;
            } else if (dyn->d_tag == DT_RELA) {
                rela = (Elf64_Rela *)((char *)base + dyn->d_un.d_ptr);
            } else if (dyn->d_tag == DT_RELASZ) {
                relasz = dyn->d_un.d_val;
            }
        }
        if (h->nsyms == 0 && h->symtab && h->strtab && syment)
            h->nsyms = ((char *)h->strtab - (char *)h->symtab) / syment;
        if (rela && relasz) {
            size_t n = relasz / sizeof(Elf64_Rela);
            for (size_t i = 0; i < n; i++) {
                if (ELF64_R_TYPE(rela[i].r_info) == R_X86_64_RELATIVE) {
                    *(Elf64_Addr *)((char *)base + rela[i].r_offset) =
                        (Elf64_Addr)((char *)base + rela[i].r_addend);
                }
            }
        }
    }

    close(fd);
    return h;
}

void *dlsym(void *handle, const char *symbol)
{
    struct dl_handle *h = handle;
    if (!h || !h->symtab || !h->strtab)
        return NULL;
    for (size_t i = 0; i < h->nsyms; i++) {
        const char *name = h->strtab + h->symtab[i].st_name;
        if (strcmp(name, symbol) == 0)
            return (char *)h->base + h->symtab[i].st_value;
    }
    return NULL;
}

int dlclose(void *handle)
{
    struct dl_handle *h = handle;
    if (!h)
        return -1;
    munmap(h->mapping, h->map_size);
    free(h);
    return 0;
}
