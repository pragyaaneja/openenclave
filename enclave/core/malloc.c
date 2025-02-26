// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/advanced/allocator.h>
#include <openenclave/corelibc/errno.h>
#include <openenclave/corelibc/stdlib.h>
#include <openenclave/internal/fault.h>
#include <openenclave/internal/globals.h>
#include <openenclave/internal/malloc.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/safecrt.h>
#include <openenclave/internal/utils.h>

static oe_allocation_failure_callback_t _failure_callback;

void oe_set_allocation_failure_callback(
    oe_allocation_failure_callback_t function)
{
    _failure_callback = function;
}

void* oe_malloc(size_t size)
{
    void* p = oe_allocator_malloc(size);

    if (!p && size)
    {
        oe_errno = OE_ENOMEM;
        if (_failure_callback)
            _failure_callback(__FILE__, __LINE__, __FUNCTION__, size);
    }
    else
        oe_errno = 0;

    return p;
}

void oe_free(void* ptr)
{
    oe_allocator_free(ptr);
}

void* oe_calloc(size_t nmemb, size_t size)
{
    void* p = oe_allocator_calloc(nmemb, size);

    if (!p && nmemb && size)
    {
        oe_errno = OE_ENOMEM;
        if (_failure_callback)
            _failure_callback(__FILE__, __LINE__, __FUNCTION__, nmemb * size);
    }
    else
        oe_errno = 0;

    return p;
}

void* oe_realloc(void* ptr, size_t size)
{
    void* p = oe_allocator_realloc(ptr, size);

    if (!p && size)
    {
        oe_errno = OE_ENOMEM;
        if (_failure_callback)
            _failure_callback(__FILE__, __LINE__, __FUNCTION__, size);
    }
    else
        oe_errno = 0;

    return p;
}

void* oe_memalign(size_t alignment, size_t size)
{
    void* ptr = NULL;

    if (!oe_is_pow2(alignment))
        oe_errno = OE_EINVAL;
    else
    {
        if (alignment < sizeof(void*))
            alignment = sizeof(void*);
        oe_errno = oe_posix_memalign(&ptr, alignment, size);
    }

    return ptr;
}

int oe_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    // Alignment must be a power of two
    if (!oe_is_pow2(alignment))
        return OE_EINVAL;

    // Alignment must be a multiple of sizeof(void*).
    // Since sizeof(void*) is a power of 2, we can just do the following check.
    if (alignment < sizeof(void*))
        return OE_EINVAL;

    int rc = oe_allocator_posix_memalign(memptr, alignment, size);

    if (rc != 0 && size)
    {
        if (_failure_callback)
            _failure_callback(__FILE__, __LINE__, __FUNCTION__, size);
    }

    return rc;
}

size_t oe_malloc_usable_size(void* ptr)
{
    return oe_allocator_malloc_usable_size(ptr);
}

// Dummy item in malloc.c for the real variables and functions in debugmalloc.c
bool oe_disable_debug_malloc_check;

oe_result_t oe_check_memory_leaks(void)
{
    // Without debug malloc, no leaks are reported.
    return OE_OK;
}

oe_result_t oe_debug_malloc_tracking_start(void)
{
    return OE_OK;
}

oe_result_t oe_debug_malloc_tracking_stop(void)
{
    return OE_OK;
}

oe_result_t oe_debug_malloc_tracking_report(
    uint64_t* out_object_count,
    char** report)
{
    *out_object_count = 0;
    *report = NULL;

    // return OE_FAILURE on purpose, to differentiate from the api in
    // debugmalloc.c
    return OE_FAILURE;
}
