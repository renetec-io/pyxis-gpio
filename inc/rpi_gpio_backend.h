/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#ifndef RPI_GPIO_BACKEND_H
#define RPI_GPIO_BACKEND_H

#define COUNTOF(a) (sizeof(a)/sizeof((a)[0]))

#include "gpio_backend.h"

int rpi_gpio_backend_init(p_gpio_backend_ops **ops);
int rpi_gpio_backend_destroy(p_gpio_backend_ops **ops);

#endif
