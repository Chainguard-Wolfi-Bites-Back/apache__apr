/* Copyright 2000-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr_env.h"
#include "apr_errno.h"
#include "testutil.h"

#define TEST_ENVVAR_NAME "apr_test_envvar"
#define TEST_ENVVAR_VALUE "Just a value that we'll check"

static int have_env_set;
static int have_env_get;

static void test_setenv(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_env_set(TEST_ENVVAR_NAME, TEST_ENVVAR_VALUE, p);
    have_env_set = (rv != APR_ENOTIMPL);
    if (!have_env_set) {
        abts_not_impl(tc, "apr_env_set");
    }
    apr_assert_success(tc, "set environment variable", rv);
}

static void test_getenv(abts_case *tc, void *data)
{
    char *value;
    apr_status_t rv;

    if (!have_env_set) {
        abts_not_impl(tc, "apr_env_set (skip test for apr_env_get)");
    }

    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    have_env_get = (rv != APR_ENOTIMPL);
    if (!have_env_get) {
        abts_not_impl(tc, "apr_env_get");
    }
    apr_assert_success(tc, "get environment variable", rv);
    abts_str_equal(tc, TEST_ENVVAR_VALUE, value);
}

static void test_delenv(abts_case *tc, void *data)
{
    char *value;
    apr_status_t rv;

    if (!have_env_set) {
        abts_not_impl(tc, "apr_env_set (skip test for apr_env_delete)");
    }

    rv = apr_env_delete(TEST_ENVVAR_NAME, p);
    if (rv == APR_ENOTIMPL) {
        abts_not_impl(tc, "apr_env_delete");
    }
    apr_assert_success(tc, "delete environment variable", rv);

    if (!have_env_get) {
        abts_not_impl(tc, "apr_env_get (skip sanity check for apr_env_delete)");
    }
    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    abts_int_equal(tc, APR_ENOENT, rv);
}

abts_suite *testenv(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_setenv, NULL);
    abts_run_test(suite, test_getenv, NULL);
    abts_run_test(suite, test_delenv, NULL);

    return suite;
}

