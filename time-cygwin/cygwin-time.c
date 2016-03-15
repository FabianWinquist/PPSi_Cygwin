/*
 * Copyright (C) 2011 CERN (www.cern.ch)
 * Author: Fabian Winquist, Omar Abdilameer
 *
 * Released to the public domain
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h> /*timex.h not available on cygwin so we use sys/time.h*/
#include <ppsi/ppsi.h>

static void clock_fatal_error(char *context)
{
	pp_error("failure in \"%s\": %s\n.Exiting.\n", context,
		  strerror(errno));
	exit(1);
}

static int cygwin_time_get(struct pp_instance *ppi, TimeInternal *t)
{
	struct timespec tp;
	if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
		clock_fatal_error("clock_gettime");
	/* TAI = UTC + 35 */
	t->seconds = tp.tv_sec + DSPRO(ppi)->currentUtcOffset;
	t->nanoseconds = tp.tv_nsec;
	if (!(pp_global_d_flags & PP_FLAG_NOTIMELOG))
		pp_diag(ppi, time, 2, "%s: %9li.%09li\n", __func__,
			tp.tv_sec, tp.tv_nsec);
	return 0;
}

static int cygwin_time_set(struct pp_instance *ppi, TimeInternal *t)
{
	struct timespec tp;

	if (!t) { /* Change the network notion of the utc/tai offset */
		pp_diag(ppi, time, 1, "New TAI offset: %i\n",
			DSPRO(ppi)->currentUtcOffset);
		return 0;
	}

	/* UTC = TAI - 35 */
	tp.tv_sec = t->seconds - DSPRO(ppi)->currentUtcOffset;
	tp.tv_nsec = t->nanoseconds;
	if (clock_settime(CLOCK_REALTIME, &tp) < 0)
		clock_fatal_error("clock_settime");
	pp_diag(ppi, time, 1, "%s: %9li.%09li\n", __func__,
		tp.tv_sec, tp.tv_nsec);
	return 0;
}

static int cygwin_time_init_servo(struct pp_instance *ppi)
{
	return 0; /* positive or negative, not -1 */
}

static int cygwin_time_adjust(struct pp_instance *ppi, long offset_ns, long freq_ppb)
{
	struct timespec t;
	int ret;
	
	/* There exist now known function for adjusting the clock frequency in either cygwin or Windows,
	If it does in the future we can change the code here. As of know adjusting calling adjust_freq returns 0. */
	if (freq_ppb) {
		if (freq_ppb > PP_ADJ_FREQ_MAX)
			freq_ppb = PP_ADJ_FREQ_MAX;
		if (freq_ppb < -PP_ADJ_FREQ_MAX)
			freq_ppb = -PP_ADJ_FREQ_MAX;
		return 0;
	}
	
	t.tv_sec = 0;
	t.tv_nsec = 0;

	if (offset_ns) {
		t.tv_sec = offset_ns / 1000000; /* seconds */
		t.tv_nsec = offset_ns - offset_ns / 1000000;  /* nanoseconds */
	}
	/* Unfortunatly the clock adjust function does not work in cygwin, therefore we use settime. 
	Which of course is not optimal since it creates horrible time spikes. */
	ret = clock_settime(CLOCK_REALTIME, &t);
	pp_diag(ppi, time, 1, "%s: %li %li\n", __func__, offset_ns, freq_ppb);
	return ret;
}

static int cygwin_time_adjust_offset(struct pp_instance *ppi, long offset_ns)
{
	return cygwin_time_adjust(ppi, offset_ns, 0);
}

static int cygwin_time_adjust_freq(struct pp_instance *ppi, long freq_ppb)
{
	return cygwin_time_adjust(ppi, 0, freq_ppb);
}

static unsigned long cygwin_calc_timeout(struct pp_instance *ppi, int millisec)
{
	struct timespec now;
	uint64_t now_ms;
	unsigned long result;

	if (!millisec)
		millisec = 1;
	clock_gettime(CLOCK_MONOTONIC, &now);
	now_ms = 1000LL * now.tv_sec + now.tv_nsec / 1000 / 1000;

	result = now_ms + millisec;
	return result ? result : 1; /* cannot return 0 */
}

struct pp_time_operations cygwin_time_ops = {
	.get = cygwin_time_get,
	.set = cygwin_time_set,
	.adjust = cygwin_time_adjust,
	.adjust_offset = cygwin_time_adjust_offset,
	.adjust_freq = cygwin_time_adjust_freq,
	.init_servo = cygwin_time_init_servo,
	.calc_timeout = cygwin_calc_timeout,
};
