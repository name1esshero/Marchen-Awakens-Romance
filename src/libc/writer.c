/* Reentrant wrapper around _write(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>
#include <sys/unistd.h>

extern int errno;

_ssize_t
_write_r (struct _reent *ptr,
	  int file,
	  const void *buf,
	  size_t len)
{
  int ret;

  errno = 0;
  if ((ret = _write (file, buf, len)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
