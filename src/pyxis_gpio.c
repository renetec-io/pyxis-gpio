/**
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/stat.h>

#include "gpio_backend.h"
#include "pyxis_gpio_comm.h"
#include "rpi_gpio_backend.h"

#define DEBUG 1

#define MAX_CLIENT 16u

#define SOCK_PATH  "/tmp/pyxis-gpio-socket"

typedef struct
{
    int server_sock;
    p_gpio_backend_ops *ops;
} pyxis_data;

static ssize_t send_replay(int fd, p_gpio_packet_type type, void *buff, size_t size)
{
    ssize_t bytes = -1;
    p_gpio_replay_head head = {
        .type = type,
        .size = size,
    };

    while ((bytes = send(fd, &head, sizeof(p_gpio_replay_head), 0)) !=
           sizeof(p_gpio_replay_head)) {
        if (errno == EAGAIN) {
            errno = 0;
            continue;
        }
        return -1;
    }

    if (size == 0) {
        return bytes;
    }

    while ((bytes = send(fd, buff, size, 0)) != (ssize_t)size) {
        if (errno == EAGAIN) {
            errno = 0;
            continue;
        }
        return -1;
    }

    return bytes;
}

static ssize_t recv_cmd(int fd, void *buff, size_t size)
{
    ssize_t bytes = -1;
    while ((bytes = recv(fd, buff, size, MSG_WAITALL)) !=
           (ssize_t)size) {
        if (errno == EAGAIN) {
            errno = 0;
            continue;
        }
        return -1;
    }
    return bytes;
}

static void close_socket_connection(struct pollfd *pfd) {
    close(pfd->fd);
    pfd->fd = -1;
    pfd->revents = 0;
    pfd->events = 0;
}

static void pyxis_handle_cmd(pyxis_data *p_d)
{
    struct pollfd pfds[MAX_CLIENT + 1u];
    for (unsigned i = 1u; i < MAX_CLIENT + 1u; i++) {
        pfds[i].fd = -1;
    }

    pfds[0].fd = p_d->server_sock;
    pfds[0].events = POLLIN;

    while (1) {
        if (poll(pfds, MAX_CLIENT + 1u, -1) == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            errx(1, "Poll call return fail, errno:%s",
                strerror(errno));
        }

        if (pfds[0].revents & (POLLOUT | POLLIN)) {
            int client_fd = accept4(pfds[0].fd, NULL, NULL, SOCK_NONBLOCK);
            if (client_fd == -1) {
                close(client_fd);
                errx(1, "ACCEPT ERROR: %s\n",  strerror(errno));
            }

            for (unsigned i = 1u; i < MAX_CLIENT + 1u; i++) {
                if (pfds[i].fd == -1) {
                    pfds[i].fd = client_fd;
                    pfds[i].events = POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;
                    break;
                }
            }
        }

        for (unsigned i = 1u; i < MAX_CLIENT + 1u; i++) {
            if (pfds[i].revents & POLLIN) {
                p_gpio_cmd_head cmd_head;
                p_gpio_cmd_data cmd_data;

                if (recv_cmd(pfds[i].fd, &cmd_head, sizeof(p_gpio_cmd_head)) == -1) {
                    close_socket_connection(&pfds[i]);
                    continue;
                }

                if (cmd_head.size > 0) {
                    if (recv_cmd(pfds[i].fd, &cmd_data, cmd_head.size) == -1) {
                        close_socket_connection(&pfds[i]);
                        continue;
                    }
                }

                switch (cmd_head.type) {
                    case P_GPIO_CMD_SET_PIN_MODE:
                    {
                        int32_t status;
                        DEBUG_PRINT("P_GPIO_CMD_SET_PIN_MODE: cmd_head.pin %u, mode %u\n",
                                    cmd_head.pin, cmd_data.pin_mode);
                        status = p_d->ops->set_mode(cmd_head.pin, cmd_data.pin_mode);
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_SET_PIN_MODE, &status, sizeof(int32_t));
                        break;
                    }
                    case P_GPIO_CMD_SET_PIN_PUD:
                    {
                        int32_t status;
                        DEBUG_PRINT("P_GPIO_CMD_SET_PIN_PUD: cmd_head.pin %u, pud %u\n",
                                    cmd_head.pin, cmd_data.pin_pud);
                        status = p_d->ops->set_pud(cmd_head.pin, cmd_data.pin_pud);
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_SET_PIN_PUD, &status, sizeof(int32_t));
                        break;
                    }
                    case P_GPIO_CMD_READ:
                    {
                        int32_t status;
                        DEBUG_PRINT("P_GPIO_CMD_READ: cmd_head.pin %u\n", cmd_head.pin);
                        status = p_d->ops->read(cmd_head.pin);
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_READ, &status, sizeof(int32_t));
                        break;
                    }
                    case P_GPIO_CMD_WRITE:
                    {
                        int32_t status;
                        DEBUG_PRINT("P_GPIO_CMD_WRITE: cmd_head.pin %u, state %u\n",
                                    cmd_head.pin, cmd_data.write_value);
                        status = p_d->ops->write(cmd_head.pin, cmd_data.write_value);
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_WRITE, &status, sizeof(int32_t));
                        break;
                    }
                    case P_GPIO_CMD_SET_PWM:
                    {
                        int32_t status;
                        DEBUG_PRINT("P_GPIO_CMD_PWM: pin %u, freq %u, range %u, duty %u\n",
                                    cmd_head.pin,
                                    cmd_data.pwm_cfg.frequency,
                                    cmd_data.pwm_cfg.range,
                                    cmd_data.pwm_cfg.duty);
                        status = p_d->ops->set_pwm(cmd_head.pin,
                                                   cmd_data.pwm_cfg.frequency,
                                                   cmd_data.pwm_cfg.range,
                                                   cmd_data.pwm_cfg.duty);
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_SET_PWM, &status, sizeof(int32_t));
                        break;
                    }
                    default:
                        send_replay(pfds[i].fd, P_GPIO_REPLAY_UNDIFINED_CMD, NULL, 0);
                }
            }

            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL | POLLRDHUP)) {
                close_socket_connection(&pfds[i]);
            }
        }
    }
}

static void pyxis_init_server(pyxis_data *p_d)
{
    int ret;
    struct sockaddr_un server_sockaddr = {0};
    p_d->server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (p_d->server_sock == -1) {
        errx(1, "SOCKET ERROR: %s",  strerror(errno));
    }

    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SOCK_PATH);

    unlink(SOCK_PATH);
    ret = bind(p_d->server_sock, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));
    if (ret == -1) {
        close(p_d->server_sock);
        errx(1, "BIND ERROR: %s",  strerror(errno));
    }

    ret = listen(p_d->server_sock, 16u);
    if (ret == -1) {
        close(p_d->server_sock);
        errx(1, "LISTEN ERROR: %s",  strerror(errno));
    }
    chmod(SOCK_PATH, 0777);
}

int main(void) {
    pyxis_data p_d;

    if (rpi_gpio_backend_init(&p_d.ops)) {
        return EXIT_FAILURE;
    }

    pyxis_init_server(&p_d);
    pyxis_handle_cmd(&p_d);

    rpi_backend_destroy(&p_d.ops);
    close(p_d.server_sock);
    return 0;
}
