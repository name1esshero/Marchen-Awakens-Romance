/* Reentrant wrapper around _close(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>

extern int errno;
extern int _close (int file);

int
_close_r (struct _reent *ptr,
	  int file)
{
  int ret;

  errno = 0;
  if ((ret = _close (file)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
