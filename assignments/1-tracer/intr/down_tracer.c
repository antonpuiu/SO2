// SPDX-License-Identifier: GPL-2.0+

#include "down_tracer.h"

static int down_entry_handler(struct kretprobe_instance *instance,
			      struct pt_regs *regs)
{
	return tracer_update_calls(instance,
				   offsetof(struct tracer_data, down_calls));
}

struct kretprobe down_probe = { .entry_handler = down_entry_handler,
				.kp.symbol_name = DOWN_TRACER_SYMBOL_NAME,
				.maxactive = MAX_ACTIVE };
