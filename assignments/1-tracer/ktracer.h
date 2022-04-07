// SPDX-License-Identifier: GPL-2.0+

/*
** ktracer.h - kernel header of kprobe based tracer.
*/

#include "linux/sched.h"
#ifndef KTRACER_H__
#define KTRACER_H__ 1

#include <linux/types.h>
#include <linux/kprobes.h>
#include <linux/hashtable.h>
#include <linux/stddef.h>

#include <asm/atomic.h>

#include "mem/mem_data.h"

#define MAX_ACTIVE 32
#define PROC_MODE 0644
#define PROCS_TABLE_SIZE 10

#define PRINT_FORMAT "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n"

#define PRINT_HASH_FORMAT "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"
#define PRINT_PID "PID"
#define PRINT_KMALLOC "kmalloc"
#define PRINT_KFREE "kfree"
#define PRINT_KMALLOC_MEM "kmalloc_mem"
#define PRINT_KFREE_MEM "kfree_mem"
#define PRINT_SCHED "sched"
#define PRINT_UP "up"
#define PRINT_DOWN "down"
#define PRINT_LOCK "lock"
#define PRINT_UNLOCK "unlock"

#define HANDLER_CONTINUE 0
#define HANDLER_STOP 1

struct tracer_data {
	struct hlist_node node;
	struct mem_data kmalloc_data;
	struct mem_data kfree_data;

	atomic_t sched_calls;
	atomic_t lock_calls;
	atomic_t unlock_calls;

	atomic_t up_calls;
	atomic_t down_calls;
	pid_t tgid;
};

extern struct hlist_head procs_table[1 << PROCS_TABLE_SIZE];
extern struct hlist_head addr_table[1 << PROCS_TABLE_SIZE];

static inline int tracer_update_calls(struct kretprobe_instance *instance,
				      ssize_t displacement)
{
	struct tracer_data *data;
	struct hlist_head *head;
	pid_t pid;

	pid = instance->task->tgid;
	head = &procs_table[hash_min(pid, HASH_BITS(procs_table))];

	hlist_for_each_entry (data, head, node) {
		if (pid == data->tgid) {
			arch_atomic_inc(
				(atomic_t *)((ssize_t)data + displacement));

			return HANDLER_CONTINUE;
		}
	}

	return HANDLER_STOP;
}

static inline struct task_struct *get_proc(pid_t pid)
{
	struct pid *pid_struct;
	struct task_struct *task;

	rcu_read_lock();
	pid_struct = find_get_pid(pid);
	task = pid_task(pid_struct, PIDTYPE_TGID);
	rcu_read_unlock();

	return task;
}

#endif // KTRACER_H__
