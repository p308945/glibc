/* Copyright (C) 2003-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sysdep.h>
#include <kernel-features.h>
#include "kernel-posix-timers.h"

int
__timer_settime64 (timer_t timerid, int flags,
                   const struct __itimerspec64 *value,
                   struct __itimerspec64 *ovalue)
{
  struct timer *kt = (struct timer *) timerid;

#ifdef __ASSUME_TIME64_SYSCALLS
# ifndef __NR_timer_settime64
#  define __NR_timer_settime64 __NR_timer_settime
# endif
  return INLINE_SYSCALL_CALL (timer_settime64, kt->ktimerid, flags, value,
                              ovalue);
#else
# ifdef __NR_timer_settime64
  int ret = INLINE_SYSCALL_CALL (timer_settime64, kt->ktimerid, flags, value,
                                 ovalue);
  if (ret == 0 || errno != ENOSYS)
    return ret;
# endif
  struct itimerspec its32, oits32;

  if (! in_time_t_range ((value->it_value).tv_sec)
      || ! in_time_t_range ((value->it_interval).tv_sec))
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  its32.it_interval = valid_timespec64_to_timespec (value->it_interval);
  its32.it_value = valid_timespec64_to_timespec (value->it_value);

  int retval = INLINE_SYSCALL_CALL (timer_settime, kt->ktimerid, flags,
                                    &its32, ovalue ? &oits32 : NULL);
  if (retval == 0 && ovalue)
    {
      ovalue->it_interval = valid_timespec_to_timespec64 (oits32.it_interval);
      ovalue->it_value = valid_timespec_to_timespec64 (oits32.it_value);
    }

  return retval;
#endif
}

#if __TIMESIZE != 64
librt_hidden_def (__timer_settime64)

int
__timer_settime (timer_t timerid, int flags, const struct itimerspec *value,
                 struct itimerspec *ovalue)
{
  struct __itimerspec64 its64, oits64;
  int retval;

  its64.it_interval = valid_timespec_to_timespec64 (value->it_interval);
  its64.it_value = valid_timespec_to_timespec64 (value->it_value);

  retval = __timer_settime64 (timerid, flags, &its64, ovalue ? &oits64 : NULL);
  if (retval == 0 && ovalue)
    {
      ovalue->it_interval = valid_timespec64_to_timespec (oits64.it_interval);
      ovalue->it_value = valid_timespec64_to_timespec (oits64.it_value);
    }

  return retval;
}
#endif
weak_alias (__timer_settime, timer_settime)
