/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef UNLOCK_TRACER_H__
#define UNLOCK_TRACER_H__ 1

#include "../ktracer.h"

#define UNLOCK_TRACER_SYMBOL_NAME "mutex_unlock"

extern struct kretprobe unlock_probe;

#endif // UNLOCK_TRACER_H__
