// SPDX-License-Identifier: GPL-2.0+

#include "linux/gfp.h"
#include "linux/personality.h"
#include "linux/pid.h"
#include "linux/rcupdate.h"
#include "linux/sched.h"
#include "linux/spinlock.h"
#include <linux/spinlock_types.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kprobes.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <asm/atomic.h>

#include "tracer.h"
#include "ktracer.h"
#include "kmodules.h"

#define PRINT_ALERT_FORMAT "%s %s\n"

#define IOCTL_ENTRY "[tracer_ioctl]"

#define NOMEM_ERR "No more memory."
#define WRONG_PID "Process not found."

DEFINE_HASHTABLE(procs_table, PROCS_TABLE_SIZE);
DEFINE_HASHTABLE(addr_table, PROCS_TABLE_SIZE);

static struct proc_dir_entry *tracer_proc;

static struct kretprobe *probes[] = { &kmalloc_probe, &kfree_probe,
				      &sched_probe,   &lock_probe,
				      &unlock_probe,  &up_probe,
				      &down_probe };

static inline void tracer_init_data(struct tracer_data *data, pid_t tgid)
{
	init_mem_data(&data->kmalloc_data);
	init_mem_data(&data->kfree_data);

	arch_atomic_set(&data->sched_calls, 0);
	arch_atomic_set(&data->lock_calls, 0);
	arch_atomic_set(&data->unlock_calls, 0);

	arch_atomic_set(&data->up_calls, 0);
	arch_atomic_set(&data->down_calls, 0);

	data->tgid = tgid;
}

static int tracer_proc_show(struct seq_file *m, void *v)
{
	int i;
	struct tracer_data *crt;

	seq_printf(m, PRINT_FORMAT, PRINT_PID, PRINT_KMALLOC, PRINT_KFREE,
		   PRINT_KMALLOC_MEM, PRINT_KFREE_MEM, PRINT_SCHED, PRINT_UP,
		   PRINT_DOWN, PRINT_LOCK, PRINT_UNLOCK);

	hash_for_each (procs_table, i, crt, node) {
		seq_printf(m, PRINT_HASH_FORMAT, crt->tgid,
			   arch_atomic_read(&crt->kmalloc_data.calls),
			   arch_atomic_read(&crt->kfree_data.calls),
			   arch_atomic_read(&crt->kmalloc_data.mem),
			   arch_atomic_read(&crt->kfree_data.mem),
			   arch_atomic_read(&crt->sched_calls),
			   arch_atomic_read(&crt->up_calls),
			   arch_atomic_read(&crt->down_calls),
			   arch_atomic_read(&crt->lock_calls),
			   arch_atomic_read(&crt->unlock_calls));
	}

	return 0;
}

static int tracer_proc_open(struct inode *inode, struct file *file)
{
	if (inode == NULL || file == NULL)
		return -EINVAL;

	return single_open(file, tracer_proc_show, NULL);
}

static inline long tracer_add_proc(unsigned long arg)
{
	struct task_struct *task;
	struct tracer_data *data;

	task = get_proc(arg);

	if (task == NULL) {
		pr_alert(PRINT_ALERT_FORMAT, IOCTL_ENTRY, WRONG_PID);

		return -ESRCH;
	}

	data = (struct tracer_data *)kmalloc(sizeof(struct tracer_data),
					     GFP_KERNEL);

	if (data == NULL) {
		pr_alert(PRINT_ALERT_FORMAT, IOCTL_ENTRY, NOMEM_ERR);

		return -ENOMEM;
	}

	tracer_init_data(data, arg);

	hash_add(procs_table, &data->node, arg);

	return 0;
}

static inline long tracer_rm_proc(unsigned long arg)
{
	struct tracer_data *data;
	int i;

	hash_for_each (procs_table, i, data, node) {
		if (data->tgid == arg) {
			hash_del(&data->node);

			kfree(data);

			return 0;
		}
	}

	pr_alert(PRINT_ALERT_FORMAT, IOCTL_ENTRY, WRONG_PID);
	return -ESRCH;
}

static long tracer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (cmd == TRACER_ADD_PROCESS)
		return tracer_add_proc(arg);
	else if (cmd == TRACER_REMOVE_PROCESS)
		return tracer_rm_proc(arg);

	return -EPERM;
}

static inline int tracer_init_watchers(void)
{
	int n = ARRAY_SIZE(probes);
	int i;
	int ret;

	for (i = 0; i < n; i++) {
		ret = register_kretprobe(probes[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static inline void tracer_remove_watchers(void)
{
	int n;
	int i;

	n = ARRAY_SIZE(probes);

	for (i = 0; i < n; i++)
		unregister_kretprobe(probes[i]);
}

static inline void tracer_clear_hashtables(void)
{
	struct tracer_data *data_t;
	int i;

	hash_for_each (procs_table, i, data_t, node) {
		kfree(data_t);
	}
}

static const struct file_operations tracer_ops = { .unlocked_ioctl =
							   tracer_ioctl };

static const struct proc_ops tracer_proc_ops = { .proc_open = tracer_proc_open,
						 .proc_read = seq_read,
						 .proc_release =
							 single_release };

static struct miscdevice tracer_device = { .minor = TRACER_DEV_MINOR,
					   .name = TRACER_DEV_NAME,
					   .fops = &tracer_ops };

static int tracer_init(void)
{
	int err;

	tracer_proc =
		proc_create(TRACER_DEV_NAME, PROC_MODE, NULL, &tracer_proc_ops);

	if (!tracer_proc)
		return -ENOMEM;

	err = misc_register(&tracer_device);
	if (err < 0)
		return err;

	tracer_init_watchers();
	hash_init(procs_table);
	hash_init(addr_table);

	return 0;
}

static void tracer_exit(void)
{
	misc_deregister(&tracer_device);
	proc_remove(tracer_proc);
	tracer_remove_watchers();
	tracer_clear_hashtables();
}

module_init(tracer_init);
module_exit(tracer_exit);

MODULE_DESCRIPTION("Linux kernel tracer");
MODULE_AUTHOR("Anton Puiu <anton.puiu@email.com>");
MODULE_LICENSE("GPL v2");
