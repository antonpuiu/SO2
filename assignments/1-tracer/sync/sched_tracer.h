// SPDX-License-Identifier: GPL-2.0+

#ifndef SCHED_TRACER_H__
#define SCHED_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>
#include <asm/atomic.h>

#include "../ktracer.h"

#define SCHED_TRACER_SYMBOL_NAME "schedule"

extern struct kretprobe sched_probe;

#endif // SCHED_TRACER_H__
