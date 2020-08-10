/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#include "rpi_gpio_backend.h"

#include <bcm2835.h>

#include <malloc.h>
#include <stdint.h>
#include <errno.h>

#define PLLD_CLOCK 500000000

static const unsigned rpi_gpio_pins[] = {2, 3, 4, 14, 15, 17, 18, 27, 22, 23, 24, 10, 9, 25, 11, 8, 7, 5, 6, 12, 13, 19, 16, 26, 20, 21};

#define RPI_GPIO_PWM_ALT0_12 12
#define RPI_GPIO_PWM_ALT0_13 13
#define RPI_GPIO_PWM_ALT5_18 18
#define RPI_GPIO_PWM_ALT5_19 19

static int rpi_set_mode(unsigned pin, p_gpio_pin_mode mode)
{
  unsigned pin_it = 0;
  for (; pin_it < COUNTOF(rpi_gpio_pins); pin_it++) {
    if (rpi_gpio_pins[pin_it] == pin) {
      break;
    }
  }

  if (pin_it == COUNTOF(rpi_gpio_pins)) {
    return -ENODEV;
  }

  if (mode == P_GPIO_INPUT) {
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
  } else if (mode == P_GPIO_OUTPUT_PP) {
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
  } else if (mode == P_GPIO_OUTPUT_OD) {
    return -ENOTSUP;
  } else if (mode == P_GPIO_OUTPUT_OS) {
    return -ENOTSUP;
  } else if (mode == P_GPIO_PWM) {
    switch (pin) {
      case RPI_GPIO_PWM_ALT0_12:
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_ALT0);
        bcm2835_pwm_set_mode(0, 1, 1);
        break;
      case RPI_GPIO_PWM_ALT0_13:
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_ALT0);
        bcm2835_pwm_set_mode(1, 1, 1);
        break;
      case RPI_GPIO_PWM_ALT5_18:
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_ALT5);
        bcm2835_pwm_set_mode(0, 1, 1);
        break;
      case RPI_GPIO_PWM_ALT5_19:
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_ALT5);
        bcm2835_pwm_set_mode(1, 1, 1);
        break;
      default:
        fprintf(stderr, "Pin number %u doesn't support pwm mode", pin);
        return -ENOTSUP;
    }
  }

  return 0;
}

static int rpi_set_pud(unsigned pin, p_gpio_pin_pud pud)
{
  unsigned pin_it = 0;
  for (; pin_it < COUNTOF(rpi_gpio_pins); pin_it++) {
    if (rpi_gpio_pins[pin_it] == pin) {
      break;
    }
  }

  if (pin_it == COUNTOF(rpi_gpio_pins)) {
    return -ENODEV;
  }

  switch (pud) {
    case P_GPIO_FLOATING:
      bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_OFF);
      break;
    case P_GPIO_PULL_UP:
      bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_UP);
      break;
    case P_GPIO_PULL_DOWN:
      bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_DOWN);
      break;
    default:
      fprintf(stderr, "Can't find type of gpio pud config.\n");
      return -EINVAL;
  }
  return 0;
}

static int rpi_read(unsigned pin, uint8_t *value)
{
  *value = bcm2835_gpio_lev(pin);
  return 0;
}

static int rpi_write(unsigned pin, uint8_t value)
{
  bcm2835_gpio_write(pin, value);
  return 0;
}

static int rpi_set_pwm(unsigned pin,
             unsigned frequency,
             unsigned range,
             unsigned duty)
{
  unsigned pwm_chanel;

  if (!frequency) {
    frequency = 1;
    duty = 0;
  }

  if (!range) {
    return -EINVAL;
  }

  unsigned real_range = ((double)PLLD_CLOCK / (2.0 * frequency)) + 0.5;
  unsigned real_duty = real_range * duty / range;

  if (real_range < range || real_duty < duty) {
    return -EINVAL;
  }

  if (pin == RPI_GPIO_PWM_ALT0_12 || pin == RPI_GPIO_PWM_ALT5_18) {
    pwm_chanel = 0;
  } else if (pin == RPI_GPIO_PWM_ALT0_13 || pin == RPI_GPIO_PWM_ALT5_19) {
    pwm_chanel = 1;
  } else {
    return -EINVAL;
  }

  bcm2835_pwm_set_data(pwm_chanel, real_duty);
  bcm2835_pwm_set_range(pwm_chanel, real_range);
  return 0;
}

int rpi_gpio_backend_init(p_gpio_backend_ops **ops)
{
  if (!bcm2835_init()) {
    return -1;
  }

  bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_2);
  *ops = malloc(sizeof(p_gpio_backend_ops));

  (*ops)->set_mode = rpi_set_mode;
  (*ops)->set_pud = rpi_set_pud;
  (*ops)->read = rpi_read;
  (*ops)->write = rpi_write;
  (*ops)->set_pwm = rpi_set_pwm;

  return 0;
}

int rpi_gpio_backend_destroy(p_gpio_backend_ops **ops)
{
  for (unsigned i = 0; i < sizeof(rpi_gpio_pins)/sizeof(unsigned); i++) {
    bcm2835_gpio_set_pud(rpi_gpio_pins[i], BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_fsel(rpi_gpio_pins[i], BCM2835_GPIO_FSEL_INPT);
  }

  free(*ops);
  bcm2835_close();
  return 0;
}
