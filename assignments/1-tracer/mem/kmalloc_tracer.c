// SPDX-License-Identifier: GPL-2.0+

#include "kmalloc_tracer.h"
#include <linux/gfp.h>
#include <linux/slab.h>

static int kmalloc_entry_handler(struct kretprobe_instance *instance,
				 struct pt_regs *regs)
{
	struct tracer_data *data_proc;
	struct addr_data *data_addr;
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;

	pid_t pid;
	pid_t tid;

	pid = instance->task->tgid;
	tid = instance->task->pid;

	head_proc = &procs_table[hash_min(pid, HASH_BITS(procs_table))];

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (pid == data_proc->tgid) {
			arch_atomic_inc(&data_proc->kmalloc_data.generic.calls);

			head_addr = &data_proc->kmalloc_data.addr_table[hash_min(
				tid,
				HASH_BITS(data_proc->kmalloc_data.addr_table))];

			hlist_for_each_entry (data_addr, head_addr, node) {
				if (tid == data_addr->tid) {
					data_addr->pending_size = regs->ax;

					return 0;
				}
			}

			/* TODO: place significant return value. */
			return 0;
		}
	}

	/* TODO: place significant return value. */
	return 0;
}

static int kmalloc_handler(struct kretprobe_instance *instance,
			   struct pt_regs *regs)
{
	struct tracer_data *data_proc;
	struct addr_data *data_addr;
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;
	void *addr;
	pid_t pid;
	pid_t tid;

	addr = (void *)regs->ax;

	pid = instance->task->tgid;
	tid = instance->task->pid;

	head_proc = &procs_table[hash_min(pid, HASH_BITS(procs_table))];

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (pid == data_proc->tgid) {
			head_addr = &data_proc->kmalloc_data.addr_table[hash_min(
				tid,
				HASH_BITS(data_proc->kmalloc_data.addr_table))];

			hlist_for_each_entry (data_addr, head_addr, node) {
				if (tid == data_addr->tid) {
					if (addr != NULL) {
						data_addr->address = addr;
						arch_atomic_add(
							data_addr->pending_size,
							&data_addr->size);
					}

					arch_atomic_add(
						arch_atomic_read(
							&data_addr->size),
						&data_proc->kmalloc_data.generic
							 .mem);
					data_addr->pending_size = 0;

					return 0;
				}
			}

			/* TODO: place significant return value. */
			return 0;
		}
	}

	return 0;
}

struct kretprobe kmalloc_probe = { .handler = kmalloc_handler,
				   .entry_handler = kmalloc_entry_handler,
				   .kp.symbol_name = KMALLOC_TRACER_SYMBOL_NAME,
				   .maxactive = MAX_ACTIVE };
