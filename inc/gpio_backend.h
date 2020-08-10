/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#ifndef GPIO_BACKEND_H
#define GPIO_BACKEND_H

#include "pyxis_gpio_comm.h"

#ifdef DEBUG
#define DEBUG_PRINT(...) do{fprintf(stderr, __VA_ARGS__);} while (0)
#else
#define DEBUG_PRINT(...) do{} while (0)
#endif

typedef struct {
  int (*set_mode)(unsigned pin, p_gpio_pin_mode mode);
  int (*set_pud)(unsigned pin, p_gpio_pin_pud pud);
  int (*read)(unsigned pin, uint8_t *value);
  int (*write)(unsigned pin, uint8_t value);
  int (*set_pwm)(unsigned pin,
                 unsigned frequency,
                 unsigned range,
                 unsigned duty);
} p_gpio_backend_ops;

#endif
