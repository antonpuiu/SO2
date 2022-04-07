/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef KFREE_TRACER_H__
#define KFREE_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>

#include "mem_data.h"

#include "../ktracer.h"

#define KFREE_TRACER_SYMBOL_NAME "kfree"

extern struct kretprobe kfree_probe;

#endif // KFREE_TRACER_H__
