/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef KMALLOC_TRACER_H__
#define KMALLOC_TRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>

#include "mem_data.h"

#include "../ktracer.h"

#define KMALLOC_TRACER_SYMBOL_NAME "__kmalloc"

extern struct kretprobe kmalloc_probe;

#endif // KMALLOC_TRACER_H__
