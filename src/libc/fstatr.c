/* Reentrant wrapper around _fstat(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>
#include <sys/stat.h>

extern int errno;

int
_fstat_r (struct _reent *ptr,
	  int fd,
	  struct stat *buf)
{
  int ret;

  errno = 0;
  if ((ret = _fstat (fd, buf)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
