// SPDX-License-Identifier: GPL-2.0+

#include "kmalloc_tracer.h"
#include <linux/gfp.h>
#include <linux/slab.h>

static int kmalloc_entry_handler(struct kretprobe_instance *instance,
				 struct pt_regs *regs)
{
	struct hlist_head *head_proc;
	struct tracer_data *data_proc;
	struct addr_data *data_addr;
	pid_t tgid;

	tgid = current->tgid;

	head_proc = &procs_table[hash_min(tgid, HASH_BITS(procs_table))];

	if (head_proc->first == NULL)
		return HANDLER_STOP;

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (data_proc->tgid == tgid) {
			arch_atomic_inc(&data_proc->kmalloc_data.calls);

			data_addr = (struct addr_data *)kmalloc(
				sizeof(struct addr_data), GFP_KERNEL);

			if (data_addr == NULL)
				return -ENOMEM;

			data_addr->tgid = tgid;
			data_addr->size = (ssize_t)regs->ax;
			data_addr->addr = 0;

			hash_add(addr_table, &data_addr->node, tgid);

			return HANDLER_CONTINUE;
		}
	}

	return HANDLER_STOP;
}

static int kmalloc_handler(struct kretprobe_instance *instance,
			   struct pt_regs *regs)
{
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;
	struct tracer_data *data_proc;
	struct addr_data *data_addr;
	unsigned long addr;
	pid_t tgid;

	addr = regs->ax;
	tgid = current->tgid;

	if (addr == 0)
		return HANDLER_STOP;

	head_proc = &procs_table[hash_min(tgid, HASH_BITS(procs_table))];

	if (head_proc->first == NULL)
		return HANDLER_STOP;

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (data_proc->tgid == tgid) {
			head_addr = &addr_table[hash_min(
				tgid, HASH_BITS(addr_table))];

			if (head_addr->first == NULL)
				return HANDLER_STOP;

			hlist_for_each_entry (data_addr, head_addr, node) {
				if (data_addr->tgid == tgid) {
					data_addr->addr = addr;
					arch_atomic_add(
						data_addr->size,
						&data_proc->kmalloc_data.mem);

					return HANDLER_CONTINUE;
				}
			}
		}
	}

	return HANDLER_STOP;
}

struct kretprobe kmalloc_probe = { .handler = kmalloc_handler,
				   .entry_handler = kmalloc_entry_handler,
				   .kp.symbol_name = KMALLOC_TRACER_SYMBOL_NAME,
				   .maxactive = MAX_ACTIVE };
