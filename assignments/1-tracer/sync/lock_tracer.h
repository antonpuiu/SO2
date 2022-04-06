// SPDX-License-Identifier: GPL-2.0+

#ifndef LOCK_TRACER_H__
#define LOCK_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>
#include <asm/atomic.h>

#include "../ktracer.h"

#define LOCK_TRACER_SYMBOL_NAME "mutex_lock_nested"

extern struct kretprobe lock_probe;
extern atomic_t lock_calls;

#endif // LOCK_TRACER_H__
