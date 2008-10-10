#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mtd-user.h"

struct cfg_head
{
	char magic[4]; /* cfg\n */
	unsigned int version;
	unsigned int reserved1;
	unsigned int reserved2;
};

static void usage(int exitval, char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "cfg.helper /dev/mtdX <cmd> [<args>]\n");
	fprintf(stderr, "Where <cmd> is:\n");
	fprintf(stderr, "\terase\t\t\tErase device\n");
	fprintf(stderr, "\tgetversion\t\tGet version\n");
	fprintf(stderr, "\tsetversion <version>\tSet version\n");
	exit(exitval);
}

static void flash_erase(int fd)
{
	mtd_info_t info;
	erase_info_t erase;

	if (ioctl(fd, MEMGETINFO, &info) != 0) {
		fprintf(stderr, "unable to get MTD device info\n");
		exit(1);
	}

	erase.start = 0;
	erase.length = info.erasesize;

	while (erase.start < info.size) {
		if (ioctl(fd, MEMERASE, &erase) != 0) {
			fprintf(stderr, "MTD Erase failure: %s\n", strerror(errno));
			break;
		}

		erase.start += erase.length;
	}

}

static int set_version(int fd, unsigned int version)
{
	struct cfg_head head, back;

	/* write header in 2 passes so we are robust against power loss during
	   write. First write everything except magic value and then write magic
	   If magic is valid, then we know that the rest of the header is also
	   correctly written. */
	memset(&head, 0xff, sizeof(head));
	head.version = version;
	write(fd, &head, sizeof(head));

	memcpy(&head.magic, "cfg\n", sizeof(head.magic));
	lseek(fd, 0, SEEK_SET);
	write(fd, &head.magic, sizeof(head.magic));

	lseek(fd, 0, SEEK_SET);
	read(fd, &back, sizeof(back));

	return memcmp(&head, &back, sizeof(head));
}

static unsigned int get_version(int fd)
{
	struct cfg_head head;

	read(fd, &head, sizeof(head));

	if (memcmp(head.magic, "cfg\n", sizeof(head.magic))
	    || head.version == 0xffffffff)
		return 0;
	else
		return head.version;
}

int main(int argc, char **argv)
{
	int fd;

	if (argc < 3)
	        usage(1, "Invalid number of arguments!");

	fd = open(argv[1], O_RDWR);
	if (fd == -1)
		usage(2, strerror(errno));

	if (!strcmp(argv[2], "erase"))
		flash_erase(fd);
	else if (!strcmp(argv[2], "getversion"))
		printf("%d\n", get_version(fd));
	else if (!strcmp(argv[2], "setversion")) {
		unsigned int version;
		char *end;

		if (argc != 4)
			usage(1, "No version given!");

		version = strtol(argv[3], &end, 0);
		if (*end)
			usage(1, "Invalid version!");

		if (set_version(fd, version)) {
			fprintf(stderr, "Verify failed, version NOT written\n");
			return 3;
		}
	} else
		usage(1, "Invalid command!");

	close(fd);

	return 0;
}
