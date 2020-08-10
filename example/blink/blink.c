/**
 * Example - how to use pyxis gpio service.
 * Copyright (c) Renetec, Inc. All rights reserved.
 */

#include "pyxis_gpio_comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <err.h>

static int send_and_handle_cmd(int fd, p_gpio_packet_type type,
                void *data, size_t size)
{
  p_gpio_reply_head reply_head;
  int32_t status;

  p_gpio_cmd_head cmd_head = {
    .type = type,
    .pin = 23u,
    .size = size,
  };

  send(fd, &cmd_head, sizeof(p_gpio_cmd_head), 0);
  send(fd, data, size, 0);
  recv(fd, &reply_head, sizeof(p_gpio_reply_head), MSG_WAITALL);
  recv(fd, &status, reply_head.size, MSG_WAITALL);
  return status;
}

static void blink(int sock_fd)
{
  int ret;
  uint8_t state = 0;

  p_gpio_pin_mode mode = P_GPIO_OUTPUT_PP;
  send_and_handle_cmd(sock_fd, P_GPIO_CMD_SET_PIN_MODE, &mode, sizeof(p_gpio_pin_mode));

  while (1) {
    state = !state;
    ret = send_and_handle_cmd(sock_fd, P_GPIO_CMD_WRITE, &state, sizeof(state));
    if (ret) {
      fprintf(stderr, "Something wrong\n");
      return;
    }
    sleep(1);
  }
}

static int connect_to_gpio_server(const char *path)
{
  int sock_fd;
  struct sockaddr_un sockaddr = {0};

  sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    errx(1, "SOCKET ERROR: %s",  strerror(errno));
  }

  sockaddr.sun_family = AF_UNIX;
  strcpy(sockaddr.sun_path, path);

  if (connect(sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
    close(sock_fd);
    errx(1, "CONNECT ERROR: %s",  strerror(errno));
  }
  return sock_fd;
}

int main(void)
{
  int sock_fd = connect_to_gpio_server(DEFAULT_GPIO_SOCK_PATH);
  blink(sock_fd);
  close(sock_fd);

  return 0;
}
