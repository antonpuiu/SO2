// SPDX-License-Identifier: GPL-2.0+

#include "kfree_tracer.h"

#include "asm/atomic.h"
#include <linux/types.h>
#include <linux/kprobes.h>

#include "../ktracer.h"

static int kfree_entry_handler(struct kretprobe_instance *instance,
			       struct pt_regs *regs)
{
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;
	struct hlist_node *i, *j;
	struct tracer_data *data_proc;
	struct addr_data *data_addr;
	unsigned long addr;
	pid_t tgid;

	addr = regs->ax;
	tgid = current->tgid;

	head_proc = &procs_table[hash_min(tgid, HASH_BITS(procs_table))];

	if (head_proc->first == NULL)
		return HANDLER_STOP;

	hlist_for_each_entry_safe(data_proc, i, head_proc, node) {
		if (data_proc->tgid == tgid) {
			arch_atomic_inc(&data_proc->kfree_data.calls);

			head_addr = &addr_table[hash_min(
				tgid, HASH_BITS(addr_table))];

			if (head_addr->first == NULL)
				return HANDLER_STOP;

			hlist_for_each_entry_safe(data_addr, j, head_addr,
						   node) {
				if (data_addr->tgid == tgid &&
				    data_addr->addr == addr) {
					arch_atomic_add(
						data_addr->size,
						&data_proc->kfree_data.mem);

					return HANDLER_CONTINUE;
				}
			}
		}
	}

	return HANDLER_STOP;
}

struct kretprobe kfree_probe = { .entry_handler = kfree_entry_handler,
				 .kp.symbol_name = KFREE_TRACER_SYMBOL_NAME,
				 .maxactive = MAX_ACTIVE };
