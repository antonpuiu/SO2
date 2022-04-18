// SPDX-License-Identifier: GPL-2.0+

#include <linux/module.h>
#include <linux/moduleparam.h>

#include "kdata.h"
#include "uart16550.h"

static int major = UART16550_MAJOR;
static int option = OPTION_BOTH;

module_param(major, int, PARAM_PERM);
module_param(option, int, PARAM_PERM);

MODULE_DESCRIPTION("UART driver");
MODULE_AUTHOR("Anton Puiu <anton.puiu@email.com>");
MODULE_LICENSE("GPL v2");

static int uart_init(void)
{
	return 0;
}

static void uart_exit(void)
{
}

module_init(uart_init);
module_exit(uart_exit);
