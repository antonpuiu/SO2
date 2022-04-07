/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef UP_TRACER_H__
#define UP_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>
#include <asm/atomic.h>

#include "../ktracer.h"

#define UP_TRACER_SYMBOL_NAME "up"

extern struct kretprobe up_probe;

#endif // UP_TRACER_H__
