/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef KDATA_H__
#define KDATA_H__ 1

#include <linux/spinlock.h>
#include <linux/kthread.h>

#define PROCS_TABLE_SIZE 10

extern struct hlist_head procs_table[1 << PROCS_TABLE_SIZE];
extern struct hlist_head addr_table[1 << PROCS_TABLE_SIZE];

#endif // KDATA_H__
