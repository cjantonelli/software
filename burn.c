#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <err.h>
#include <malloc.h>
#include <signal.h>
#include <pthread.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>

//  burn t cpu seconds on c cores while holding m GB virtual memory
//  of which r% is resident


void
usage(char *name)
{
	fprintf(stderr,\
	"Usage: %s [-c cores] [-m memory] [-r percent] [-t secs]\
	\n\t-c c\tconsume c cores, default 1\
	\n\t-m m\tconsume m GB, default 1\
	\n\t-r r\tforce r%% resident memory, default 0%%\
	\n\t-t t\tconsume t seconds, default 60\n",\
	name);
}


void *
loop(void *arg)

{
	for (;;)
		;
}


void
handler(int sig)
{
	exit(0);
}


int
main(int argc, char **argv)
{
	struct rlimit rl = {0, 0};
	size_t memtot = 0, memsiz;
	char *p, *myname = basename(argv[0]);
	unsigned int secs = 60;
	int i, opt, cores = 1, mem = 1, touch = 0;
	pthread_t t;

	// parse arguments
	while ((opt = getopt(argc, argv, "c:hm:r:t:?")) != -1) {
		switch (opt) {
		case 'c':
			if ((i = atoi(optarg)) > 0)
				cores = i;
			break;
		case 'm':
			if ((i = atoi(optarg)) > 0)
				mem = i;
			break;
		case 'r':
			if ((i = atoi(optarg)) < 0)
				touch = 0;
			else if (i > 100)
				touch = 100;
			else
				touch = i;
			break;
		case 't':
			if ((i = atoi(optarg)) >= 0)
				secs = i;
			break;
		default:
			usage(argv[0]);
			exit(EXIT_FAILURE);
               }
	}
	printf("%s:  burning for %d sec on %d cores holding %d GB %d%% resident memory\n", argv[0], secs, cores, mem, touch);

	// suppress any attempted core dumps
	if (setrlimit(RLIMIT_CORE, &rl) < 0)
		warn("setrlimit");

	// allocate requested virtual memory
	memsiz = mem * (size_t)1024*1024*1024;
	if (!(p = malloc(memsiz)))
		errx(1, "malloc(%lld) failed", memsiz);

	// bring requested percentage of virtual memory into physical memory
	// by writing one byte into that percentage of the allocated pages of
	// virtual memory, which page faults those pages into physical memory.
	// in the absence of swapping, those pages should remain in physical
	// memory, but no attempt is made to enforce this.
	if (touch)
		for (i = 0; i<(memsiz/getpagesize())*touch/100; i++, p += getpagesize())
			*p = i & 0xff;

	// start the requested number of cores' worth of threads, less one,
	// each of which is bound to a core and executes an endless loop.
	for (i = 1; i < cores; i++)
		if (pthread_create(&t, NULL, &loop, NULL))
			err(1, "pthread_create");

	// set an alarm signal handler, then set an alarm to go off the
	// requested number of seconds from now.  the alarm signal invokes
	// the handler, which simply exits.
	if (signal(SIGALRM, handler))
		err(1, "signal");
	if (alarm(secs) < 0)
		err(1, "alarm");

	// until the alarm goes off, the main thread executes the same
	// endless loop, consuming the remaining core.
	loop(NULL);
}
