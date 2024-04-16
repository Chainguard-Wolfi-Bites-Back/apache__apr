/*    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 */

#include "apr.h"
#include "apr_lib.h"
#if 0
#define APR_WANT_STDIO
#define APR_WANT_STRFUNC
#endif
#include "apr_want.h"
#include "apr_buffer.h"
#include "apr_encode.h"
#include "apr_strings.h"
#include "apr_private.h"


APR_DECLARE(apr_status_t) apr_buffer_mem_set(apr_buffer_t *buf,
                                             void *mem, apr_size_t len)
{

    if (len > APR_INT64_MAX) {
        return APR_EINVAL;
    }

    buf->d.mem = mem;
    buf->size = len;

    return APR_SUCCESS;
}

APR_DECLARE(apr_buffer_t *) apr_buffer_mem_make(apr_pool_t *pool,
                                                void *mem, apr_size_t len)
{
    apr_buffer_t *buf;

    if (len > APR_INT64_MAX) {
        return NULL;
    }

    buf = apr_palloc(pool, sizeof(apr_buffer_t));

    if (buf) {
        buf->d.mem = mem;
        buf->size = len;
    }

    return buf;
}

APR_DECLARE(apr_status_t) apr_buffer_str_set(apr_buffer_t *buf,
                                             char *str, apr_ssize_t len)
{

    if (len > APR_INT64_MAX) {
        return APR_EINVAL;
    }

    if (!str) {
        buf->d.str = NULL;
        buf->size = 0;
    }
    else if (len < 0) {
        apr_size_t slen = strlen(str);
        if (slen <= APR_INT64_MAX) {
            buf->d.str = str;
            buf->size = -(slen + 1);
        }
        else {
            buf->d.str = NULL;
            buf->size = 0;
        }
    }
    else {
        buf->d.str = str;
        buf->size = -(len + 1);
    }

    return APR_SUCCESS;
}

APR_DECLARE(apr_buffer_t *) apr_buffer_str_make(apr_pool_t *pool,
                                                char *str, apr_ssize_t len)
{
    apr_buffer_t *buf;
    apr_int64_t size;
    apr_size_t slen;

    if (!str) {
        str = NULL;
        size = 0;
    }
    if (APR_BUFFER_STRING == len && !str[0]) {
        size = len;
    }
    else if (len < 0) {
        slen = strlen(str);
        if (slen <= APR_INT64_MAX) {
            size = -(slen + 1);
        }
        else {
            return NULL;
        }
    }
    else {
        size = -(len + 1);
    }

    buf = apr_palloc(pool, sizeof(apr_buffer_t));

    if (buf) {
        buf->d.str = str;
        buf->size = size;
    }

    return buf;
}

APR_DECLARE(apr_buffer_t *) apr_buffer_null_make(apr_pool_t *pool)
{
    return apr_pcalloc(pool, sizeof(apr_buffer_t));
}

APR_DECLARE(apr_size_t) apr_buffer_len(const apr_buffer_t *buf)
{
    if (buf->size < 0) {
        return (apr_size_t)((-buf->size) - 1);
    }
    else {
        return (apr_size_t)buf->size;
    }
}

APR_DECLARE(apr_size_t) apr_buffer_allocated(const apr_buffer_t *buf)
{   
    if (buf->size < 0) {
        return (apr_size_t)((-buf->size));
    }
    else {
        return (apr_size_t)buf->size;
    }
}

APR_DECLARE(int) apr_buffer_is_null(const apr_buffer_t *buf)
{
    if (!buf->d.mem) {
        return 1;
    }
    else {
        return 0;
    }
}

APR_DECLARE(int) apr_buffer_is_str(const apr_buffer_t *buf)
{
    if (buf->size < 0) {
        return 1;
    }
    else {  
        return 0;                         
    }
}

APR_DECLARE(char *) apr_buffer_str(const apr_buffer_t *buf)
{
    if (buf->size < 0) {
        return buf->d.str;
    }
    else {  
        return NULL;
    }
}

APR_DECLARE(char *) apr_buffer_pstrdup(apr_pool_t *pool, const apr_buffer_t *buf)
{
    if (buf->size < 0) {
        return apr_pstrmemdup(pool, buf->d.str, -(buf->size + 1));
    }
    else {
        return apr_pstrmemdup(pool, buf->d.str, buf->size);
    }
}

APR_DECLARE(void *) apr_buffer_mem(const apr_buffer_t *buf, apr_size_t *size)
{
    if (size) {
        size[0] = apr_buffer_len(buf);
    }

    return buf->d.mem;
}

APR_DECLARE(void *) apr_buffer_pmemdup(apr_pool_t *pool, const apr_buffer_t *buf, apr_size_t *size)
{
    apr_size_t len = apr_buffer_len(buf);

    if (size) {
        size[0] = len;
    }

    return apr_pmemdup(pool, buf->d.mem, len);
}

APR_DECLARE(apr_buffer_t *) apr_buffer_arraydup(const apr_buffer_t *src,
                                                apr_buffer_alloc alloc, void *ctx,
                                                int nelts)
{
    apr_buffer_t *dst = alloc(ctx, nelts * sizeof(apr_buffer_t));
    apr_buffer_t *d = dst;

    int i;
    for (i = 0; i < nelts; i++) {

        /* absolute value is size of mem buffer including optional terminating zero */
        apr_int64_t size = src->size < 0 ? -src->size : src->size;

        void *mem = alloc(ctx, size);
        memcpy(mem, src->d.mem, size);

        dst->size = src->size;
        dst->d.mem = mem;

        src++;
        dst++;
    }

    return d;
}

APR_DECLARE(apr_buffer_t *) apr_buffer_dup(const apr_buffer_t *buf,
                                           apr_buffer_alloc alloc, void *ctx)
{
    return apr_buffer_arraydup(buf, alloc, ctx, 1);
}

APR_DECLARE(apr_buffer_t *) apr_buffer_cpy(apr_buffer_t *dst,
                                           const apr_buffer_t *src,
                                           apr_buffer_alloc alloc, void *ctx)
{
    if (!src) {

        dst->d.mem = NULL;
        dst->size = 0;

    }
    else if (!alloc) {

        dst->d.mem = src->d.mem;
        dst->size = src->size;
    
    }
    else {

        /* absolute value is size of mem buffer including optional terminating zero */
        apr_int64_t size = src->size < 0 ? -src->size : src->size;

        void *mem = alloc(ctx, size);
        memcpy(mem, src->d.mem, size);

        dst->size = src->size;
        dst->d.mem = mem;

    }

    return dst;
}


APR_DECLARE(int) apr_buffer_cmp(const apr_buffer_t *src,
                                const apr_buffer_t *dst)
{
    apr_size_t slen = apr_buffer_len(src);
    apr_size_t dlen = apr_buffer_len(dst);
   
    if (slen != dlen) {
        return slen < dlen ? -1 : 1;
    }
    else if (src->d.mem == dst->d.mem) {
        /* identical data or both NULL */
        return 0;
    }
    else if (!src->d.mem) {
        return -1;
    }
    else if (!dst->d.mem) {
        return 1;
    }
    else {
        return memcmp(src->d.mem, dst->d.mem, slen);
    }
}


APR_DECLARE(int) apr_buffer_ncmp(const apr_buffer_t *src,
                                 const apr_buffer_t *dst)
{
    if (!src || !src->d.mem) {
        if (!dst || !dst->d.mem) {
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        if (!dst || !dst->d.mem) {
            return 1;
        }
        else {
            return apr_buffer_cmp(src, dst);
        }
    }
}

APR_DECLARE(char *) apr_buffer_pstrncat(apr_pool_t *p, const apr_buffer_t *buf,
                                        int nelts, const char *sep, int flags,
                                        apr_size_t *nbytes)
{
    const apr_buffer_t *src = buf;
    apr_size_t seplen = sep ? strlen(sep) : 0;
    apr_size_t size = 0;

    char *dst, *str;

    int i;
    for (i = 0; i < nelts; i++) {

        if (i > 0) {
            size += seplen;
        }

        if (src->size < 0) {
            size += (-src->size) - 1;
        }
        else {
            if (APR_BUFFER_NONE == flags) {
                size += src->size;
            }
            else if (APR_BUFFER_BASE64 == flags) {
                apr_size_t b64len;

                if (APR_SUCCESS != apr_encode_base64(NULL, src->d.mem, src->size,
                                                     APR_ENCODE_NONE, &b64len)) {
                    return NULL;
                }
                size += b64len - 1;
            }
        }

        src++;
    }

    if (nbytes) {
        *nbytes = size;
    }

    str = dst = apr_palloc(p, size + 1);

    src = buf;

    for (i = 0; i < nelts; i++) {

        if (i > 0 && sep) {
            strncpy(dst, sep, seplen);
            dst += seplen;
        }
        
        if (src->size < 0) {
            strncpy(dst, src->d.str, (-src->size) - 1);
            dst += (-src->size) - 1;
        }
        else {
            if (APR_BUFFER_NONE == flags) {
                memcpy(dst, src->d.mem, src->size);
            }
            else if (APR_BUFFER_BASE64 == flags) {
                apr_size_t b64len;

                if (APR_SUCCESS != apr_encode_base64(dst, src->d.mem, src->size,
                                                     APR_ENCODE_NONE, &b64len)) {
                    return NULL;
                }
                dst += b64len;
            }
        }

        src++;
    }

    dst[0] = 0;

    return str;
}

