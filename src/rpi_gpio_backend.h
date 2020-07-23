/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#ifndef RPI_GPIO_BACKEND_H
#define RPI_GPIO_BACKEND_H

#include "gpio_backend.h"

int rpi_gpio_backend_init(p_gpio_backend_ops **ops);
int rpi_backend_destroy(p_gpio_backend_ops **ops);

#endif
