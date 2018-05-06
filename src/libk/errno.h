#ifndef ERRNO_H
#define ERRNO_H

/* ERRNO
 *
 * Upper 16 bits are the error number
 * Lower 16 bits are the additional information
 *
 * i.e.
 * errno = EACCESS;
 * errno = ECUSTOMERROR | 83;
 */
int errno;
#endif
