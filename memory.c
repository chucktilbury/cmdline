/**
 * @file memory.c
 * 
 * @brief This is a wrapper for the standard memory function plus a couple 
 * that are intended to make it easier to make copies of strings and other 
 * objects. These functions are intended to be called through the macros in
 * the header file.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-10
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "myassert.h"

/**
 * @brief Allocate memory using malloc() and check for errors.
 * 
 * @param size 
 * @return void* 
 */
void* mem_alloc(size_t size) {

    void* ptr = malloc(size);
    ASSERT_MSG(ptr != NULL, "cannot allocate %lu bytes", size);

    memset(ptr, 0, size);
    return ptr;
}

/**
 * @brief Reallocate memory using standard system calls and check for errors.
 * 
 * @param ptr 
 * @param size 
 * @return void* 
 */
void* mem_realloc(void* ptr, size_t size) {

    void* nptr = realloc(ptr, size);
    ASSERT_MSG(nptr != NULL, "cannot reallocate %lu bytes", size);

    return nptr;
}

/**
 * @brief Copy an area of memory into a new allocated buffer with the given 
 * size.
 * 
 * @param ptr 
 * @param size 
 * @return void* 
 */
void* mem_copy(void* ptr, size_t size) {

    void* nptr = mem_alloc(size);
    ASSERT(nptr != NULL);

    memcpy(nptr, ptr, size);
    return nptr;
}

/**
 * @brief Make a copy of a string into a new memory buffer. Note that strdup() 
 * has been depricated by ISO.
 * 
 * @param str 
 * @return const char* 
 */
const char* mem_copy_str(const char* str) {

    if(str == NULL || strlen(str) == 0)
        return mem_alloc(1);
    else
        return (const char*)mem_copy((void*)str, strlen(str));
}

/**
 * @brief Free a memory buffer using the normal system call.
 * 
 * @param ptr 
 */
void mem_free(void* ptr) {

    if(ptr != NULL)
        free(ptr);
}
