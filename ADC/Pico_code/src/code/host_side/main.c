#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define BUFSIZE (64*1024) 


static volatile sig_atomic_t stop_requested = 0;

static void sigint_handler(int sig) {
    (void)sig;
    stop_requested = 1;   /* async-signal-safe: set a flag only */
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s /dev/ttyACM0 out.bin\n", argv[0]);
        return 1;
    }
    const char *dev = argv[1];
    const char *out = argv[2];

    /* Open device: O_RDWR asserts DTR on most systems (useful if firmware waits for DTR).
       If your firmware doesn't need DTR, O_RDONLY is fine. */
    int fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) { perror("open device"); return 1; }

	struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; /* don't use SA_RESTART so read() is interrupted */
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
	
	struct termios tio;
    if (tcgetattr(fd, &tio) != 0) { close(fd); return 1; }
    cfmakeraw(&tio);            /* raw mode so bytes are untouched */
    tio.c_cc[VMIN] = 1;         /* return as soon as at least 1 byte */
    tio.c_cc[VTIME] = 0;
   	if (tcsetattr(fd, TCSANOW, &tio) != 0) { close(fd); return 1; }

    /* Open output file */
    int outfd = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (outfd < 0) {
        perror("open out");
        tcsetattr(fd, TCSANOW, &tio);
        close(fd);
        return 1;
    }
	static unsigned char buf[BUFSIZE];

    char start = 'S';
    ssize_t w;
    do {
        w = write(fd, &start, 1);
    } while (w < 0 && errno == EINTR && !stop_requested);

    if (w < 0) {
        perror("write start");
        goto cleanup;
    }
    while (1) {
        ssize_t n = read(fd, buf, BUFSIZE);
        if (n < 0) {
            if (errno == EINTR) {
                if (stop_requested) break; /* Ctrl-C pressed -> exit loop */
                continue;                  /* otherwise retry */
            }
            perror("read");
            break;
        } else if (n == 0) {
            /* Device closed / EOF */
            break;
        }

        ssize_t written = 0;
        while (written < n) {
            ssize_t m = write(outfd, buf + written, (size_t)(n - written));
            if (m < 0) {
                if (errno == EINTR) {
                    if (stop_requested) break; /* stop if interrupted by Ctrl-C */
                    continue;                  /* otherwise retry write */
                }
                perror("write");
                goto cleanup;
            }
            written += m;
        }
    }

cleanup:
    close(outfd);
    close(fd);
	perror("code exited");
    return 0;
}
