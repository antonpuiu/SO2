/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef DOWN_TRACER_H__
#define DOWN_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>
#include <asm/atomic.h>

#include "../ktracer.h"

#define DOWN_TRACER_SYMBOL_NAME "down_interruptible"

extern struct kretprobe down_probe;

#endif // DOWN_TRACER_H__
