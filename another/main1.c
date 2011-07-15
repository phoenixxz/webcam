#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>           

#include <fcntl.h>             
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>         
#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
    void *                  start;
    size_t                  length;
};

static char *           dev_name        = "/dev/video0";
static int              fd              = -1;
struct buffer *         buffers         = NULL;

FILE *file_fd;
static unsigned long file_length;
static unsigned char *file_name;

int main (int argc,char ** argv)
{
    struct v4l2_capability cap; 
    struct v4l2_format fmt;

    file_fd = fopen("test.jpg", "w");

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    ioctl (fd, VIDIOC_QUERYCAP, &cap);

    CLEAR (fmt);
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640; 
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    ioctl (fd, VIDIOC_S_FMT, &fmt);

    file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

    buffers = calloc (1, sizeof (*buffers));

    buffers[0].length = file_length;
    buffers[0].start = malloc (file_length);

    for (;;) 
    {
	fd_set fds;
	struct timeval tv;
	int r;

	FD_ZERO (&fds);
	FD_SET (fd, &fds);

	/* Timeout. */
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	r = select (fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == r) {
	    if (EINTR == errno)
		continue;
	    printf ("select");
	}

	if (0 == r) {
	    fprintf (stderr, "select timeout/n");
	    exit (EXIT_FAILURE);
	}

	if (read (fd, buffers[0].start, buffers[0].length))
	    break;
    }

    fwrite(buffers[0].start, buffers[0].length, 1, file_fd);

    free (buffers[0].start);
    close (fd);
    fclose (file_fd);
    exit (EXIT_SUCCESS);
    return 0;
}
