/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef KDATA_H
#define KDATA_H

#define UART16550_MAJOR 42
#define COM1_MAJOR 42
#define COM2_MAJOR 42

#define MINOR_COM1 0
#define MINOR_COM2 1

#define IRQ_COM1 4
#define IRQ_COM2 3

#define PARAM_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#endif // KDATA_H
