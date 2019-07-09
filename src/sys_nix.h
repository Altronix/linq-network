#ifndef SYS_NIX_H_
#define SYS_NIX_H_

#define linq_malloc_fn malloc
#define linq_free_fn free

#define linq_malloc(x) linq_malloc_fn(x)
#define linq_free(x) linq_free_fn(x)

#endif /* SYS_NIX_H_ */
