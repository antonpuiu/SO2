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

struct addr_data {
	pid_t tgid;
	unsigned long addr;
	ssize_t size;
	struct hlist_node node;
};

static inline void init_mem_data(struct mem_data *data)
{
	arch_atomic_set(&data->calls, 0);
	arch_atomic_set(&data->mem, 0);
}

#endif // MEM_DATA_H__
