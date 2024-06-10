/**
 * @file memory.h
 * 
 * @brief Public interface for memory wrappers.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-10
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>

#define _ALLOC(s)                   mem_alloc(s)
#define _ALLOC_DS(t)                (t*)mem_alloc(sizeof(t))
#define _ALLOC_DS_ARRAY(t, n)       (t*)mem_alloc(sizeof(t)*(n))
#define _REALLOC(p, s)              mem_realloc((void*)(p), (s))
#define _REALLOC_DS(p, t)           (t*)mem_realloc((void*)(p), sizeof(t))
#define _REALLOC_DS_ARRAY(p, t, n)  (t*)mem_realloc((void*)(p), sizeof(t)*(n))
#define _COPY(p, s)                 mem_copy((void*)(p), (s))
#define _COPY_STR(str)              mem_copy_str(str)
#define _COPY_DS(p, t)              (t*)mem_copy((void*)(p), sizeof(t))
#define _COPY_DS_ARRAY(p, t)        (t*)mem_copy((void*)(p), sizeof(t)*(n))
#define _FREE(p)                    mem_free((void*)(p))

void* mem_alloc(size_t size);
void* mem_realloc(void* ptr, size_t size);
void* mem_copy(void* ptr, size_t size);
const char* mem_copy_str(const char* str);
void mem_free(void* ptr);

#endif  /* _MEMORY_H_ */
