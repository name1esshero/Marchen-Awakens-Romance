/* Reentrant wrapper around _sbrk(). Propagates a nonzero global errno
   into the caller's reentrancy struct when the underlying syscall fails. */

#include <reent.h>
#include <sys/unistd.h>

extern int errno;

void *
_sbrk_r (struct _reent *ptr,
	 size_t incr)
{
  void *ret;

  errno = 0;
  if ((ret = _sbrk (incr)) == (void *) -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}
