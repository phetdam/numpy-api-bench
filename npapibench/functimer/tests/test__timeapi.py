"""Unit tests for timing functions provided by the _timeapi extension.

.. codeauthor:: Derek Huang <djh458@stern.nyu.edu>
"""

import numpy as np
import pytest

# pylint: disable=no-name-in-module,relative-beyond-top-level
from .._timeapi import autorange, timeit_plus, timeit_once, timeit_repeat
from .._timeunit import MAX_PRECISION


@pytest.fixture(scope="session")
def timeargs():
    """Default function and positional args to use with timing functions.

    Returns
    -------
    tuple
    """
    return max, (1, 2)


def test_timeit_once_sanity(pytype_raise, pyvalue_raise, timeargs):
    """Sanity checks for _timeapi.timeit_once.

    Don't need to check if args is tuple and if kwargs is dict since
    PyArg_ParseTupleAndKeywords handles this for us.

    Parameters
    ----------
    pytype_raise : function
        pytest fixture. See top-level package conftest.py.
    pyvalue_raise : function
        pytest fixture. See top-level package conftest.py.
    timeargs : tuple
        pytest fixture. See timeargs.
    """
    # require callable func and timer (if timer provided)
    with pyvalue_raise("func must be callable"):
        timeit_once("not callable")
    with pyvalue_raise("timer must be callable"):
        timeit_once(*timeargs, timer=22)
    # timer must return a float value and not take arguments
    with pyvalue_raise("timer must return a float starting value"):
        timeit_once(*timeargs, timer=lambda: None)
    with pytype_raise():
        timeit_once(*timeargs, timer=lambda x: x)
    # number of function calls in the trial must be positive
    with pyvalue_raise("number must be positive"):
        timeit_once(*timeargs, number=0)


def test_timeit_once_number(timeargs):
    """Test that number is used correctly in _timeapi.timeit_once.

    Parameters
    ----------
    timeargs : tuple
        pytest fixture. See timeargs.
    """
    # values of number to pass. time should increase as number increases
    numbers = [10, 1000, 10000, 100000, 1000000]
    # compute times for each value of number
    times = np.empty(5)
    for i, number in enumerate(numbers):
        times[i] = timeit_once(*timeargs, number=number)
    # check that times are in ascending order
    assert np.all(times[:-1] < times[1:])


def test_autorange(timeargs):
    """Test that _timeapi.autorange returns values as expected.

    Don't need to check if args is tuple and if kwargs is dict since
    PyArg_ParseTupleAndKeywords handles this for us. timeit_once is called
    internally and will do checks for func, timer.

    Parameters
    ----------
    timeargs : tuple
        pytest fixture. See timeargs.
    """
    # max is a fast function so we should expect result divisible by 10
    assert autorange(*timeargs) % 10 == 0


def test_timeit_repeat_sanity(pyvalue_raise, timeargs):
    """Sanity checks for _timeapi.timeit_repeat.

    Don't need to check if args is tuple and if kwargs is dict since
    PyArg_ParseTupleAndKeywords handles this for us. timeit_once is called
    internally and will do checks for func, timer, number.

    Parameters
    ----------
    pyvalue_raise : function
        pytest fixture. See top-level package conftest.py.
    timeargs : tuple
        pytest fixture. See timeargs.
    """
    # repeat must be positive
    with pyvalue_raise(match="repeat must be positive"):
        timeit_repeat(*timeargs, repeat=0)


@pytest.mark.skip(reason="not yet refactored")
def test_timeit_plus_sanity(pyvalue_raise, timeargs):
    """Sanity checks for _timeapi.timeit_plus.

    Don't need to check if args is tuple and if kwargs is dict since
    PyArg_ParseTupleAndKeywords handles this for us. Don't need to check timer,
    number, repeat since timeit_repeat is called and will do checks for these.

    Parameters
    ----------
    pyvalue_raise : function
        pytest fixture. See top-level package conftest.py.
    timeargs : tuple
        pytest fixture. See timeargs.
    """
    # number must be positive
    with pyvalue_raise(match="number must be positive"):
        timeit_plus(*timeargs, number=0)
    # repeat must be positive
    with pyvalue_raise(match="repeat must be positive"):
        timeit_plus(*timeargs, repeat=0)
    # unit must be valid
    with pyvalue_raise(match="unit must be one of"):
        timeit_plus(*timeargs, unit="bloops")
    # precision must be positive and less than TimeitResult.MAX_PRECISION
    with pyvalue_raise(match="precision must be positive"):
        timeit_plus(*timeargs, precision=0)
    with pytest.raises(
        ValueError,
        match=f"precision is capped at {MAX_PRECISION}"
    ):
        timeit_plus(*timeargs, precision=MAX_PRECISION + 1)
    # warning will be raised if precision >= TimeitResult.MAX_PRECISION // 2
    with pytest.warns(UserWarning, match="precision is rather high"):
        timeit_plus(*timeargs, precision=MAX_PRECISION // 2)
    # this should run normally
    tir = timeit_plus(*timeargs)
    print(tir.brief)