/*
 * Copyright (C) 2011 CERN (www.cern.ch)
 * Author: Fabian Winquist, Omar Abdilameer
 *
 * Released to the public domain
 */

/*
 * These are the functions provided by the various cygwin files
 */

#define POSIX_ARCH(ppg) ((struct cygwin_arch_data *)(ppg->arch_data))
struct cygwin_arch_data {
	struct timeval tv;
};

extern void cygwin_main_loop(struct pp_globals *ppg);
