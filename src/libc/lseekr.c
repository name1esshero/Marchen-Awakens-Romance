/* Reentrant wrapper around _lseek(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>
#include <sys/unistd.h>

extern int errno;

_off_t
_lseek_r (struct _reent *ptr,
	  int file,
	  _off_t offset,
	  int dir)
{
  int ret;

  errno = 0;
  if ((ret = _lseek (file, offset, dir)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
