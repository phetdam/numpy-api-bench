/**
 * @file timeitresult_suite.c
 * @brief Creates test suite for the `c_npy_demp.functimer.TimeitResult` class.
 */

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include <stdio.h>

#include <check.h>

#include "test_helpers.h"
#include "timeitresult.h"
#include "timeitresult_suite.h"

// empty macros indicating whether test needs Python C API or not.
#define PY_C_API_REQUIRED
#define NO_PY_C_API

/**
 * Python interpreter fixture setup to allow use of the Python C API
 */
void py_setup(void) {
  Py_Initialize();
}

/**
 * Python interpreter fixture teardown to finalize interpreter
 */
void py_teardown(void) {
  Py_FinalizeEx_handle_err()
}

/**
 * Test that `TimeitResult_validate_unit` works as expected.
 */
NO_PY_C_API START_TEST(test_validate_unit) {
  // return false if arg is NULL
  ck_assert_msg(
    !TimeitResult_validate_unit(NULL), "%s: TimeitResult_validate_unit should "
    "return false if passed NULL pointer", __func__
  );
  // foobar is not a valid unit
  ck_assert_msg(
    !TimeitResult_validate_unit("foobar"), "%s: TimeitResult_validate_unit "
    "should not validate invalid unit \"foobar\"", __func__
  );
  // nsec is a valid unit
  ck_assert_msg(
    TimeitResult_validate_unit("nsec"), "%s: TimeitResult_validate_unit "
    "should validate valid unit \"nsec\"", __func__
  );
}

/**
 * Test that `TimeitResult_dealloc` raises appropriate exceptions.
 */
PY_C_API_REQUIRED START_TEST(test_dealloc) {
  // call and get borrowed reference to exception type
  TimeitResult_dealloc(NULL);
  PyObject *exc = PyErr_Occurred();
  // this pointer should not be NULL (exception was set)
  ck_assert_ptr_nonnull(exc);
  // check that exc is of type RuntimeError
  ck_assert_msg(
    PyErr_GivenExceptionMatches(exc, PyExc_RuntimeError),
    "%s: TimeitResult_dealloc should set RuntimeError if given NULL pointer",
    __func__
  );
}

/**
 * Test that `TimeitResult_new` is argument safe for extern access.
 * 
 * @note Technically C API module functions should be static.
 */
PY_C_API_REQUIRED START_TEST(test_new) {
  // dummy tuple needed to pass to args
  PyObject *args = PyTuple_New(0);
  // if NULL, test fails
  if (args == NULL) {
    fprintf(stderr, "error: %s: unable to allocate size-0 tuple\n", __func__);
    return;
  }
  // should set exception if first arg is NULL. exc is borrowed.
  TimeitResult_new(NULL, args, NULL);
  PyObject *exc = PyErr_Occurred();
  // exc should not be NULL
  ck_assert_ptr_nonnull(exc);
  // check that exc is RuntimeError
  ck_assert_msg(
    PyErr_GivenExceptionMatches(exc, PyExc_RuntimeError),
    "%s: TimeitResult_new should set RuntimeError if type is NULL", __func__
  );
  // should set exception if args is NULL. exc is borrowed. note the very
  // unsafe cast that is done for the type argument.
  TimeitResult_new((PyTypeObject *) args, NULL, NULL);
  exc = PyErr_Occurred();
  // exc should not be NULL and should be RuntimeError
  ck_assert_ptr_nonnull(exc);
  ck_assert_msg(
    PyErr_GivenExceptionMatches(exc, PyExc_RuntimeError),
    "%s: TimeitResult_new should set RuntimeError if args is NULL", __func__
  );
  // Py_DECREF tuple since we don't need it anymore
  Py_DECREF(args);
}

/**
 * Create test suite `"timeitresult_suite"` using static tests defined above.
 * 
 * Invokes unit tests for `TimeitResult` in two cases. The first case,
 * `"py_core"` uses the `py_setup` and `py_teardown` functions to set up an
 * unchecked fixture (runs in same address space and only at start and end of
 * test case). The second case, `"c_core"`, doesn't use the Python C API.
 * 
 * @param timeout `double` number of seconds for the test case's timeout
 * @returns libcheck `Suite *`, `NULL` on error
 */
Suite *make_timeitresult_suite(double timeout) {
  // if timeout is nonpositive, print error and return NULL
  if (timeout <= 0) {
    fprintf(stderr, "error: %s: timeout must be positive\n", __func__);
    return NULL;
  }
  // create suite called test_suite
  Suite *suite = suite_create("timeitresult_suite");
  // test case that contains unit tests that require Python C API
  TCase *tc_py_core = tcase_create("py_core");
  // test case that contains unit tests that don't require Python C API
  TCase *tc_c_core = tcase_create("c_core");
  // set test case timeouts to timeout
  tcase_set_timeout(tc_py_core, timeout);
  tcase_set_timeout(tc_c_core, timeout);
  // add py_setup and py_teardown to tc_py_core test case (required)
  tcase_add_checked_fixture(tc_py_core, py_setup, py_teardown);
  // register cases together with tests, add cases to suite, and return suite
  tcase_add_test(tc_py_core, test_dealloc);
  tcase_add_test(tc_py_core, test_new);
  tcase_add_test(tc_c_core, test_validate_unit);
  suite_add_tcase(suite, tc_py_core);
  suite_add_tcase(suite, tc_c_core);
  return suite;
}