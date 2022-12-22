#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#define V4L2_CID_LED1_MODE 0x0a046d05

void help(char *progname, int exitcode) {
  printf("Usage: %s {-g|-s [on|off|blink|auto]} DEVNAME\n",
         progname);
  free(progname);
  exit(exitcode);
}

int main(int argc, char *argv[]) {
  char *progname = malloc(strlen(argv[0]) + 1);
  strcpy(progname, argv[0]);
  enum {
    LED_OFF = 0,
    LED_ON = 1,
    LED_BLINK = 2,
    LED_AUTO = 3
  } mode = LED_AUTO;
  enum {
    CMD_UNK = -1,
    CMD_GET = VIDIOC_G_CTRL,
    CMD_SET = VIDIOC_S_CTRL} cmd = CMD_UNK;
  int opt;
  while ((opt = getopt(argc, argv, "gs:h")) != -1) {
    switch (opt) {
    case 'h':
      help(progname, EXIT_SUCCESS);
      break;
    case 'g':
      cmd = CMD_GET;
      break;
    case 's':
      cmd = CMD_SET;
      if (strcasecmp(optarg, "off") == 0) {
        mode = LED_OFF;
      } else if (strcasecmp(optarg, "on") == 0) {
        mode = LED_ON;
      } else if (strcasecmp(optarg, "blink") == 0) {
        mode = LED_BLINK;
      } else if (strcasecmp(optarg, "auto") == 0) {
        mode = LED_AUTO;
      }
      break;
    default:
      help(progname, EXIT_FAILURE);
      break;
    }
  }
  free(progname);

  if (cmd == CMD_UNK) {
    puts("Please specify action (-g to get value, -s to set)");
    exit(EXIT_FAILURE);
  }

  if (optind != argc - 1) {
    puts("Please specify exactly one video device (/dev/videoN)");
    exit(EXIT_FAILURE);
  }

  int fd = open(argv[optind], O_RDWR);
  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  struct v4l2_control ctrl;
  ctrl.id = V4L2_CID_LED1_MODE;
  ctrl.value = mode;

  int rc = ioctl(fd, cmd, &ctrl);
  if (rc == -1) {
    perror("ioctl");
    exit(EXIT_FAILURE);
  }
  close(fd);
  if (cmd == CMD_GET) {
    char *msg;
    switch (ctrl.value) {
    case LED_OFF:   msg = "off";     break;
    case LED_ON:    msg = "on";      break;
    case LED_BLINK: msg = "blink";   break;
    case LED_AUTO:  msg = "auto";    break;
    default:        msg = "unknown"; break;
    }
    printf("LED1 Mode = %s\n", msg);
  }
  return EXIT_SUCCESS;
}
