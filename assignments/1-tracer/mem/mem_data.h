// SPDX-License-Identifier: GPL-2.0+

#include "asm-generic/errno-base.h"
#include "linux/gfp.h"
#include "linux/sched/signal.h"
#include "linux/spinlock.h"
#include "linux/spinlock_types.h"
#include "linux/types.h"
#include <linux/string.h>
#ifndef MEM_DATA_H__
#define MEM_DATA_H__ 1

#include <asm/atomic.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/sched.h>

#define ADDR_TABLE_SIZE 5

struct mem_data {
	atomic_t calls;
	atomic_t mem;
};

struct mem_addr_data {
	pid_t tid;
	void *address;
	struct hlist_node node;
};

struct addr_data {
	pid_t tid;
	atomic_t size;
	ssize_t pending_size;
	void *address;
	struct hlist_node node;
};

struct kmem_data {
	struct mem_data generic;
	DECLARE_HASHTABLE(addr_table, ADDR_TABLE_SIZE);
};

static inline void init_mem_data(struct mem_data *data)
{
	arch_atomic_set(&data->calls, 0);
	arch_atomic_set(&data->mem, 0);
}

static inline int init_mem_addr_data(struct kmem_data *table,
				     struct task_struct *leader)
{
	/* TODO: implement. with mem_addr_data instead of addr_data. */
	return 0;
}

static inline int init_addr_data(struct kmem_data *table,
				 struct task_struct *leader)
{
	struct list_head *i;
	struct task_struct *crt;
	struct addr_data *data_addr;

	list_for_each (i, &leader->children) {
		crt = list_entry(i, struct task_struct, sibling);
		data_addr = (struct addr_data *)kmalloc(
			sizeof(struct addr_data), GFP_KERNEL);

		if (data_addr == NULL)
			return -ENOMEM;

		arch_atomic_set(&data_addr->size, 0);
		data_addr->tid = crt->pid;
		data_addr->pending_size = 0;
		data_addr->address = NULL;

		hash_add(table->addr_table, &data_addr->node, crt->pid);
	}

	return 0;
}

static inline void init_kmem_data(struct kmem_data *data,
				  struct task_struct *task, bool mem_addr)
{
	init_mem_data(&data->generic);
	hash_init(data->addr_table);
	if (mem_addr)
		init_mem_addr_data(data, task->group_leader);
	else
		init_addr_data(data, task->group_leader);
}

static inline void destroy_kmem_data(struct kmem_data *data)
{
	struct addr_data *data_a;
	int i;

	hash_for_each (data->addr_table, i, data_a, node) {
		kfree(data_a);
	}
}

#endif // MEM_DATA_H__
