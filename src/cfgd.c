/* cfg monitoring daemon */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>

/* in seconds */
#define DEFAULT_TIMEOUT 10

static int force;

static void sighup_handler(int nr)
{
	force = 1;
}

static void usage(const char *errmsg)
{
	if (errmsg)
		fprintf(stderr, "Error: %s\n", errmsg);
	fprintf(stderr, "Cfg commit daemon, (C) 2009 Peter Korsgaard "
		"<jacmet@sunsite.dk>\n");
	fprintf(stderr, "Usage: cfgd <dir> [<timeout>]\n");
	fprintf(stderr, "where:\n");
	fprintf(stderr, "\t<dir>\t\tDirectory to monitor for changes\n");
	fprintf(stderr, "\t<timeout>\tDelay in seconds to wait after change "
		"(default: %d)\n", DEFAULT_TIMEOUT);
	exit(errmsg ? 1 : 0);
}

int main(int argc, char **argv)
{
	struct pollfd pfd;
	int timeout = DEFAULT_TIMEOUT, changed = 0;

	if (argc > 3)
		usage("Invalid number of arguments");

	if ((argc < 2) || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
	    usage(0);

	if (argc == 3)
		timeout = atoi(argv[2]);
	timeout *= 1000;
	if (!timeout)
		usage("Invalid timeout value");

	pfd.fd = inotify_init();
	pfd.events = POLLIN;
	if (inotify_add_watch(pfd.fd, argv[1], IN_MODIFY | IN_ATTRIB | IN_MOVE |
			      IN_CREATE | IN_DELETE) == -1)
		usage("Inotify error, check dir");

	if (fcntl(pfd.fd, F_SETFL, fcntl(pfd.fd, F_GETFL) | O_NONBLOCK)) {
		perror("fcntl");
		return 2;
	}

	if (signal(SIGHUP, sighup_handler) == SIG_ERR) {
		perror("signal");
		return 3;
	}

	while (1) {
		int n, len;

		n = poll(&pfd, 1, changed ? timeout : -1);

		/* files changed? */
		if (n == 1) {
			/* empty request queue */
			do {
				char buf[1024];
				len = read(pfd.fd, buf, sizeof(buf));
			} while (len >= 0);
			changed = 1;
		} else if (force || (changed && !n)) {
			/* commit */
			system("cfg commit 2>&1 | logger -t cfg -p daemon.info");
			force = changed = 0;
		}
	}

	close(pfd.fd);
	return 0;
}
