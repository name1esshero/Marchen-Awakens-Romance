/* Reentrant wrapper around _read(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>
#include <sys/unistd.h>

extern int errno;

_ssize_t
_read_r (struct _reent *ptr,
	 int file,
	 void *buf,
	 size_t len)
{
  int ret;

  errno = 0;
  if ((ret = _read (file, buf, len)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
