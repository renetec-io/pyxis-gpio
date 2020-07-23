/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#ifndef GPIO_PYXIS_COMMUNICATION_H
#define GPIO_PYXIS_COMMUNICATION_H

#include <stdint.h>

/**
 * @brief Command for communication between pyxis-gpio service and clients
 */
typedef enum {
    // data: gpio_cmd_head + gpio_pin_mode
    P_GPIO_CMD_SET_PIN_MODE = 1u,
    // data: gpio_cmd_head + gpio_pin_pud
    P_GPIO_CMD_SET_PIN_PUD,
    // data: gpio_cmd_head
    P_GPIO_CMD_DEINIT_PIN,
    // data: gpio_cmd_head
    P_GPIO_CMD_READ,
    // data: gpio_cmd_head + uint32_t pin
    P_GPIO_CMD_WRITE,
    // data: gpio_cmd_head + gpio_pwm_config_data
    P_GPIO_CMD_SET_PWM,

    // data: gpio_replay_head + int32_t status
    P_GPIO_REPLAY_SET_PIN_MODE = 1024u,
    // data: gpio_replay_head + int32_t status
    P_GPIO_REPLAY_SET_PIN_PUD,
    // data: gpio_replay_head + int32_t status
    P_GPIO_REPLAY_READ,
    // data: gpio_replay_head + int32_t status
    P_GPIO_REPLAY_WRITE,
    // data: gpio_replay_head + int32_t status
    P_GPIO_REPLAY_SET_PWM,
    // data: gpio_replay_head
    P_GPIO_REPLAY_UNDIFINED_CMD,
} p_gpio_packet_type;

typedef enum {
    P_GPIO_LOW = 0,
    P_GPIO_HIGH,
} p_gpio_level;

typedef enum {
    P_GPIO_INPUT = 1u,
    P_GPIO_OUTPUT_PP,
    P_GPIO_OUTPUT_OD,
    P_GPIO_OUTPUT_OS,
    P_GPIO_PWM,
} p_gpio_pin_mode;

typedef enum {
    P_GPIO_FLOATING = 1u,
    P_GPIO_PULL_UP,
    P_GPIO_PULL_DOWN,
} p_gpio_pin_pud;

/**
 * @brief Head for cmd between pyxis-gpio service and clients.
 */
typedef struct {
    p_gpio_packet_type type;
    uint16_t size;
    uint16_t pin;
} p_gpio_cmd_head;

/**
 * @brief Head for replay between pyxis-gpio service and clients.
 */
typedef struct {
    p_gpio_packet_type type;
    uint16_t size;
} p_gpio_replay_head;

/**
 * @brief Data for GPIO_CMD_SET_PWM cmd.
 */
typedef struct {
    uint32_t frequency;
    uint32_t range;
    uint32_t duty;
} p_gpio_pwm_config_data;

/**
 * @brief Union for handling all types of received data.
 */
typedef union {
    p_gpio_pin_mode pin_mode;
    p_gpio_pin_pud pin_pud;
    p_gpio_pwm_config_data pwm_cfg;
    uint32_t write_value;
} p_gpio_cmd_data;

#endif /* GPIO_COMMUNICATION_H */
