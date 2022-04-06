// SPDX-License-Identifier: GPL-2.0+
#include "sched_tracer.h"

static int sched_entry_handler(struct kretprobe_instance *instance,
			       struct pt_regs *regs)
{
	return tracer_update_calls(instance,
				   offsetof(struct tracer_data, sched_calls));
}

struct kretprobe sched_probe = { .entry_handler = sched_entry_handler,
				 .kp.symbol_name = SCHED_TRACER_SYMBOL_NAME,
				 .maxactive = MAX_ACTIVE };
