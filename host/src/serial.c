#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"

static int maple_serial_fd = -1;

int maple_serial_open(const char *portname)
{
  struct termios toptions;
  int fd;

  maple_serial_close();
  fd = open(portname, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(portname);
    return -1;
  }
  if (tcgetattr(fd, &toptions) < 0) {
    perror("tcgetattr");
    close(fd);
    return -1;
  }
  toptions.c_cflag &= ~(PARENB|CSTOPB|CSIZE|CRTSCTS);
  toptions.c_cflag |= CS8|CREAD|CLOCAL;
  toptions.c_iflag &= ~(IXON|IXOFF|IXANY|ICANON|ECHO|ECHOE|ISIG);
  toptions.c_oflag &= ~OPOST;
  toptions.c_cc[VMIN] = 1;
  toptions.c_cc[VTIME] = 5;
  cfsetispeed(&toptions, B9600); /* Change speed from 1200bps to prevent */
  cfsetospeed(&toptions, B9600); /* hangup on close */
  if (tcsetattr(fd, TCSANOW, &toptions) < 0) {
    perror("tcsetattr");
    close(fd);
    return -1;
  }
  if (tcflush(fd, TCIOFLUSH) < 0) {
    perror("tcflush");
    close(fd);
    return -1;
  }
  maple_serial_fd = fd;
  return 0;
}

void maple_serial_close(void)
{
  int fd = maple_serial_fd;
  if (fd >= 0) {
    maple_serial_fd = -1;
    close(fd);
  }
}

ssize_t maple_serial_write(const uint8_t *data, size_t len)
{
  ssize_t t = 0, r;
  while (len > 0) {
    r = write(maple_serial_fd, data, len);
    if (r < 0)
      return r;
    t += r;
    if (r == 0 || r > len)
      break;
    data += r;
    len -= r;
  }
  return t;
}

ssize_t maple_serial_read(uint8_t *data, size_t len)
{
  ssize_t t = 0, r;
  while (len > 0) {
    r = read(maple_serial_fd, data, len);
    if (r < 0)
      return r;
    t += r;
    if (r == 0 || r > len)
      break;
    data += r;
    len -= r;
  }
  return t;
}
