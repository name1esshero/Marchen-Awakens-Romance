/* No-op malloc lock hooks. This target has no threads to lock against. */

struct _reent;

void
__malloc_lock (struct _reent *ptr)
{
}

void
__malloc_unlock (struct _reent *ptr)
{
}
