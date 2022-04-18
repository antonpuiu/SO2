// SPDX-License-Identifier: GPL-2.0+

/*
 * list.c - Linux kernel list API
 *
 * Author: Anton Puiu <anton.puiu@email.com>
 */
#include "linux/gfp.h"
#include "linux/stddef.h"
#include "linux/types.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include <linux/string.h>

#define PROCFS_MAX_SIZE 512

#define procfs_dir_name "list"
#define procfs_file_read "preview"
#define procfs_file_write "management"

#define PRINT_FORMAT "%s"

#define ADD_TOP_CMD "addf"
#define ADD_END_CMD "adde"
#define DEL_FST_CMD "delf"
#define DEL_ALL_CMD "dela"

#define COMMAND_LENGTH 4
#define SPACE_LENGTH 1

struct proc_dir_entry *proc_list;
struct proc_dir_entry *proc_list_read;
struct proc_dir_entry *proc_list_write;

struct so2_list {
	struct list_head list;
	char data[PROCFS_MAX_SIZE];
};

LIST_HEAD(so2_list);

static int list_proc_show(struct seq_file *m, void *v)
{
	struct list_head *i, *tmp;
	struct so2_list *sle;

	list_for_each_safe(i, tmp, &so2_list) {
		sle = list_entry(i, struct so2_list, list);
		seq_printf(m, PRINT_FORMAT, sle->data);
	}

	return 0;
}

static int list_read_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_write_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_add_cmd(struct list_head *list, char *local_buffer)
{
	struct so2_list *sle;

	sle = kmalloc(sizeof(struct so2_list), GFP_KERNEL);

	if (!sle)
		return -ENOMEM;

	strncpy(sle->data, local_buffer + COMMAND_LENGTH + SPACE_LENGTH,
		PROCFS_MAX_SIZE);
	list_add(&sle->list, list);

	return 0;
}

static void list_del_cmd(struct list_head *list, char *local_buffer, bool first)
{
	struct so2_list *sle;
	struct list_head *i, *n;

	list_for_each_safe(i, n, list) {
		sle = list_entry(i, struct so2_list, list);

		if (strcmp(sle->data,
			   local_buffer + COMMAND_LENGTH + SPACE_LENGTH) == 0) {
			list_del(i);
			kfree(sle);

			if (first)
				return;
		}
	}
}

static ssize_t list_write(struct file *file, const char __user *buffer,
			  size_t count, loff_t *offs)
{
	char local_buffer[PROCFS_MAX_SIZE];
	unsigned long local_buffer_size = 0;
	int err;

	local_buffer_size = count;
	if (local_buffer_size > PROCFS_MAX_SIZE)
		local_buffer_size = PROCFS_MAX_SIZE;

	memset(local_buffer, 0, PROCFS_MAX_SIZE);
	if (copy_from_user(local_buffer, buffer, local_buffer_size))
		return -EFAULT;

	if (strncmp(local_buffer, ADD_TOP_CMD, COMMAND_LENGTH) == 0) {
		err = list_add_cmd(&so2_list, local_buffer);

		if (err != 0)
			return err;
	} else if (strncmp(local_buffer, ADD_END_CMD, COMMAND_LENGTH) == 0) {
		err = list_add_cmd(so2_list.prev, local_buffer);

		if (err != 0)
			return err;
	} else if (strncmp(local_buffer, DEL_FST_CMD, COMMAND_LENGTH) == 0) {
		list_del_cmd(&so2_list, local_buffer, true);
	} else if (strncmp(local_buffer, DEL_ALL_CMD, COMMAND_LENGTH) == 0) {
		list_del_cmd(&so2_list, local_buffer, false);
	}

	return local_buffer_size;
}

static const struct proc_ops r_pops = {
	.proc_open = list_read_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

static const struct proc_ops w_pops = {
	.proc_open = list_write_open,
	.proc_write = list_write,
	.proc_release = single_release,
};

static int list_init(void)
{
	proc_list = proc_mkdir(procfs_dir_name, NULL);
	if (!proc_list)
		return -ENOMEM;

	proc_list_read =
		proc_create(procfs_file_read, 0000, proc_list, &r_pops);
	if (!proc_list_read)
		goto proc_list_cleanup;

	proc_list_write =
		proc_create(procfs_file_write, 0000, proc_list, &w_pops);
	if (!proc_list_write)
		goto proc_list_read_cleanup;

	return 0;

proc_list_read_cleanup:
	proc_remove(proc_list_read);
proc_list_cleanup:
	proc_remove(proc_list);
	return -ENOMEM;
}

static void list_exit(void)
{
	proc_remove(proc_list);
}

module_init(list_init);
module_exit(list_exit);

MODULE_DESCRIPTION("Linux kernel list API");
MODULE_AUTHOR("Anton Puiu <anton.puiu@email.com>");
MODULE_LICENSE("GPL v2");
