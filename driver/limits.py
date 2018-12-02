# -*- coding: utf-8 -*-

from __future__ import division, print_function

from . import returncodes
from . import util

import math
try:
    import resource
except ImportError:
    resource = None
import sys


"""
Notes on limits: On Windows, the resource module does not exist and hence we
cannot enforce any limits there. Furthermore, while the module exists on macOS,
memory limits are not enforced by that OS and hence we do not support imposing
memory limits there.
"""

CANNOT_LIMIT_MEMORY_MSG = "Setting memory limits is not supported on your platform."
CANNOT_LIMIT_TIME_MSG = "Setting time limits is not supported on your platform."


def can_set_time_limit():
    return resource is not None


def can_set_memory_limit():
    return resource is not None and sys.platform != "darwin"


def _get_soft_and_hard_time_limits(internal_limit, external_hard_limit):
    soft_limit = min(int(math.ceil(internal_limit)), external_hard_limit)
    hard_limit = min(soft_limit + 1, external_hard_limit)
    print("Time limit %.2f -> (soft: %d, hard: %d)" %
        (internal_limit, soft_limit, hard_limit))
    sys.stdout.flush()
    assert soft_limit <= hard_limit
    return soft_limit, hard_limit


def set_time_limit(time_limit):
    if time_limit is None:
        return
    if not can_set_time_limit():
        raise NotImplementedError(CANNOT_LIMIT_TIME_MSG)
    # Don't try to raise the hard limit.
    _, external_hard_limit = resource.getrlimit(resource.RLIMIT_CPU)
    if external_hard_limit == resource.RLIM_INFINITY:
        external_hard_limit = float("inf")
    if time_limit > external_hard_limit:
        raise ValueError(
            "Time limit {time_limit}s exceeds external hard limit "
            "{external_hard_limit}s.".format(**locals()))
    # Soft limit reached --> SIGXCPU.
    # Hard limit reached --> SIGKILL.
    soft_limit, hard_limit = _get_soft_and_hard_time_limits(
        time_limit, external_hard_limit)
    resource.setrlimit(resource.RLIMIT_CPU, (soft_limit, hard_limit))


def set_memory_limit(memory):
    """*memory* must be given in bytes or None."""
    if memory is None:
        return
    if not can_set_memory_limit():
        raise NotImplementedError(CANNOT_LIMIT_MEMORY_MSG)
    resource.setrlimit(resource.RLIMIT_AS, (memory, memory))


def convert_to_mb(num_bytes):
    return num_bytes / (1024 * 1024)


def get_memory_limit(component_limit, overall_limit):
    """
    Return the minimum of the component and overall limits or None if neither is set.
    """
    limits = [limit for limit in [component_limit, overall_limit] if limit is not None]
    return min(limits) if limits else None


def get_time_limit(component_limit, overall_limit):
    """
    Return the minimum time limit imposed by the component and overall limits.
    """
    limit = component_limit
    if overall_limit is not None:
        try:
            elapsed_time = util.get_elapsed_time()
        except NotImplementedError:
            returncodes.exit_with_driver_unsupported_error(CANNOT_LIMIT_TIME_MSG)
        else:
            remaining_time = max(0, overall_limit - elapsed_time)
            if limit is None or remaining_time < limit:
                limit = remaining_time
    return limit
