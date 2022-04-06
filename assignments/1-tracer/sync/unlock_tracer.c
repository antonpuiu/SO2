// SPDX-License-Identifier: GPL-2.0+

#include "unlock_tracer.h"
#include "linux/stddef.h"

static int unlock_entry_handler(struct kretprobe_instance *instance,
				struct pt_regs *regs)
{
	return tracer_update_calls(instance,
				   offsetof(struct tracer_data, unlock_calls));
}

struct kretprobe unlock_probe = { .entry_handler = unlock_entry_handler,
				  .kp.symbol_name = UNLOCK_TRACER_SYMBOL_NAME,
				  .maxactive = MAX_ACTIVE };
