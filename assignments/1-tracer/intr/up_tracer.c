// SPDX-License-Identifier: GPL-2.0+

#include "up_tracer.h"

#include <linux/types.h>
#include <linux/kprobes.h>

#include "../ktracer.h"

static int up_entry_handler(struct kretprobe_instance *instance,
			    struct pt_regs *regs)
{
	return tracer_update_calls(instance,
				   offsetof(struct tracer_data, up_calls));
}

struct kretprobe up_probe = { .entry_handler = up_entry_handler,
			      .kp.symbol_name = UP_TRACER_SYMBOL_NAME,
			      .maxactive = MAX_ACTIVE };
