// SPDX-License-Identifier: GPL-2.0+

#include "lock_tracer.h"

static int lock_entry_handler(struct kretprobe_instance *instance,
			      struct pt_regs *regs)
{
	return tracer_update_calls(instance,
				   offsetof(struct tracer_data, lock_calls));
}

struct kretprobe lock_probe = { .entry_handler = lock_entry_handler,
				.kp.symbol_name = LOCK_TRACER_SYMBOL_NAME,
				.maxactive = MAX_ACTIVE };
