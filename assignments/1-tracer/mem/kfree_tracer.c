// SPDX-License-Identifier: GPL-2.0+

#include "kfree_tracer.h"

#include "asm/atomic.h"
#include <linux/types.h>
#include <linux/kprobes.h>

#include "../ktracer.h"

static int kfree_entry_handler(struct kretprobe_instance *instance,
			       struct pt_regs *regs)
{
	struct tracer_data *data_proc;
	struct mem_addr_data *data_addr;
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;
	void *addr;
	pid_t pid;
	pid_t tid;

	pid = instance->task->tgid;
	tid = instance->task->pid;

	addr = (void *)regs->ax;

	head_proc = &procs_table[hash_min(pid, HASH_BITS(procs_table))];

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (pid == data_proc->tgid) {
			arch_atomic_inc(&data_proc->kfree_data.generic.calls);

			head_addr = &data_proc->kfree_data.addr_table[hash_min(
				tid,
				HASH_BITS(data_proc->kfree_data.addr_table))];

			hlist_for_each_entry (data_addr, head_addr, node) {
				if (tid == data_addr->tid) {
					data_addr->address = addr;

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

static int kfree_handler(struct kretprobe_instance *instance,
			 struct pt_regs *regs)
{
	struct tracer_data *data_proc;
	struct mem_addr_data *data_addr;
	struct addr_data *data_kmalloc;
	struct hlist_head *head_proc;
	struct hlist_head *head_addr;
	struct hlist_head *head_kmalloc;

	pid_t pid;
	pid_t tid;

	void *addr;

	pid = instance->task->tgid;
	tid = instance->task->pid;

	head_proc = &procs_table[hash_min(pid, HASH_BITS(procs_table))];

	hlist_for_each_entry (data_proc, head_proc, node) {
		if (pid == data_proc->tgid) {
			head_addr = &data_proc->kfree_data.addr_table[hash_min(
				tid,
				HASH_BITS(data_proc->kfree_data.addr_table))];

			hlist_for_each_entry (data_addr, head_addr, node) {
				addr = data_addr->address;
				head_kmalloc =
					&data_proc->kmalloc_data.addr_table[hash_min(
						tid,
						HASH_BITS(data_proc->kmalloc_data
								  .addr_table))];

				hlist_for_each_entry (data_kmalloc,
						      head_kmalloc, node) {
					if (data_kmalloc->address == addr) {
						arch_atomic_add(
							arch_atomic_read(
								&data_kmalloc
									 ->size),
							&data_proc->kfree_data
								 .generic.mem);

						return 0;
					}
				}
			}
		}
	}

	/* TODO: place significant return value. */
	return 0;
}

struct kretprobe kfree_probe = { .entry_handler = kfree_entry_handler,
				 .handler = kfree_handler,
				 .kp.symbol_name = KFREE_TRACER_SYMBOL_NAME,
				 .maxactive = MAX_ACTIVE };
