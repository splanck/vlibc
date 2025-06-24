/*
 * BSD 2-Clause License: Redistribution and use in source and binary forms, with or without modification, are permitted provided that the copyright notice and this permission notice appear in all copies. This software is provided "as is" without warranty.
 *
 * Purpose: Implements the error functions for vlibc. Provides wrappers and helpers used by the standard library.
 *
 * Copyright (c) 2025
 */

#include "stdio.h"
#include "errno.h"

struct err_entry {
    int code;
    const char *msg;
};

const struct err_entry __vlibc_err_table[] = {
    { EPERM, "Operation not permitted" },
    { ENOENT, "No such file or directory" },
    { ESRCH, "No such process" },
    { EINTR, "Interrupted system call" },
    { EIO, "I/O error" },
    { ENXIO, "No such device or address" },
    { E2BIG, "Argument list too long" },
    { ENOEXEC, "Exec format error" },
    { EBADF, "Bad file descriptor" },
    { ECHILD, "No child processes" },
    { EAGAIN, "Resource temporarily unavailable" },
    { ENOMEM, "Out of memory" },
    { EACCES, "Permission denied" },
    { EFAULT, "Bad address" },
    { ENOTBLK, "Block device required" },
    { EBUSY, "Device or resource busy" },
    { EEXIST, "File exists" },
    { EXDEV, "Invalid cross-device link" },
    { ENODEV, "No such device" },
    { ENOTDIR, "Not a directory" },
    { EISDIR, "Is a directory" },
    { EINVAL, "Invalid argument" },
    { ENFILE, "Too many open files in system" },
    { EMFILE, "Too many open files" },
    { ENOTTY, "Inappropriate ioctl for device" },
    { ETXTBSY, "Text file busy" },
    { EFBIG, "File too large" },
    { ENOSPC, "No space left on device" },
    { ESPIPE, "Illegal seek" },
    { EROFS, "Read-only file system" },
    { EMLINK, "Too many links" },
    { EPIPE, "Broken pipe" },
    { EDOM, "Numerical argument out of domain" },
    { ERANGE, "Numerical result out of range" },
    { EDEADLK, "Resource deadlock avoided" },
    { ENAMETOOLONG, "File name too long" },
    { ENOLCK, "No locks available" },
    { ENOSYS, "Function not implemented" },
    { ENOTEMPTY, "Directory not empty" },
    { ELOOP, "Too many levels of symbolic links" },
    { ENOMSG, "No message of desired type" },
    { EIDRM, "Identifier removed" },
    { ENOSTR, "Device not a stream" },
    { ENODATA, "No data available" },
    { ETIME, "Timer expired" },
    { ENOSR, "Out of streams resources" },
    { ENOTSOCK, "Socket operation on non-socket" },
    { EDESTADDRREQ, "Destination address required" },
    { EMSGSIZE, "Message too long" },
    { EPROTOTYPE, "Protocol wrong type for socket" },
    { ENOPROTOOPT, "Protocol not available" },
    { EPROTONOSUPPORT, "Protocol not supported" },
    { ESOCKTNOSUPPORT, "Socket type not supported" },
    { EOPNOTSUPP, "Operation not supported" },
    { EPFNOSUPPORT, "Protocol family not supported" },
    { EAFNOSUPPORT, "Address family not supported" },
    { EADDRINUSE, "Address already in use" },
    { EADDRNOTAVAIL, "Cannot assign requested address" },
    { ENETDOWN, "Network is down" },
    { ENETUNREACH, "Network is unreachable" },
    { ENETRESET, "Network dropped connection" },
    { ECONNABORTED, "Software caused connection abort" },
    { ECONNRESET, "Connection reset by peer" },
    { ENOBUFS, "No buffer space available" },
    { EISCONN, "Transport endpoint is already connected" },
    { ENOTCONN, "Transport endpoint is not connected" },
    { ETIMEDOUT, "Connection timed out" },
    { ECONNREFUSED, "Connection refused" },
    { EHOSTUNREACH, "No route to host" },
    { EALREADY, "Operation already in progress" },
    { EINPROGRESS, "Operation in progress" },
    { ESTALE, "Stale file handle" },
    { ECANCELED, "Operation canceled" },
    { 0, NULL }
};

char *strerror(int errnum)
{
    for (size_t i = 0; __vlibc_err_table[i].msg; ++i) {
        if (__vlibc_err_table[i].code == errnum)
            return (char *)__vlibc_err_table[i].msg;
    }
    static char unknown[32];
    snprintf(unknown, sizeof(unknown), "Unknown error %d", errnum);
    return unknown;
}

void perror(const char *s)
{
    char *msg = strerror(errno);
    if (s && *s)
        fprintf(stderr, "%s: %s\n", s, msg);
    else
        fprintf(stderr, "%s\n", msg);
}
