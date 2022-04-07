// SPDX-License-Identifier: GPL-2.0+

#include "mem_allocator.h"
#include "linux/gfp.h"

struct alloc_event *get_next_event(void)
{
	struct alloc_event *e;

	spin_lock(&events_lock);
	e = list_first_entry(&events_list, struct alloc_event, head);

	if (e)
		list_del(&e->head);

	spin_unlock(&events_lock);

	return e;
}

int mem_allocator_f(void *data)
{
	struct alloc_event *e;

	while (true) {
		wait_event(wq, (e = get_next_event()));

		if (e == NULL)
			continue;

		e->addr = kmalloc(e->size, GFP_KERNEL);
		arch_atomic_set(&e->solved, true);

		if (arch_atomic_read(&e->stop) == true)
			break;
	}

	do_exit(0);
}
