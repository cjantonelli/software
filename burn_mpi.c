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

#include <mpi.h>

//  burn s cpu seconds in t threads while holding m GB virtual memory
//  of which r% is resident per MPI rank


void
usage(char *name)
{
	fprintf(stderr,\
	"Usage: %s [-m memory] [-r percent] [-s secs] [-t threads]\
	\n\t-m m\tconsume m GB, default 1\
	\n\t-r r\tforce r%% resident memory, default 0%%\
	\n\t-s s\tconsume s seconds, default 60\
	\n\t-t t\tconsume t threads, default 1\n",\
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
	MPI_Finalize();
	exit(0);
}


#define	mem	tv[0]
#define	touch	tv[1]
#define	secs	tv[2]
#define	threads	tv[3]

int
main(int argc, char **argv)
{
	struct rlimit rl = {0, 0};
	size_t memtot = 0, memsiz;
	char *p, *myname = basename(argv[0]);
	int i, opt;
	pthread_t t;

	int numnodes, myid, tv[4] = {1, 0, 60, 1};

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numnodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	// parse arguments in master rank
	if (!myid) {
		while ((opt = getopt(argc, argv, "hm:r:s:t:?")) != -1) {
			switch (opt) {
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
			case 's':
				if ((i = atoi(optarg)) >= 0)
					secs = i;
				break;
			case 't':
				if ((i = atoi(optarg)) > 0)
					threads = i;
				break;
			default:
				usage(myname);
				MPI_Finalize();
				exit(EXIT_FAILURE);
		       }
		}
	}


	// send arguments to all worker ranks
	MPI_Bcast(tv, sizeof(tv)/sizeof(tv[0]), MPI_INT, 0, MPI_COMM_WORLD);
	printf("%s:  rank %d burning for %d sec on %d threads with %d GB %d%% resident memory\n", myname, myid, secs, threads, mem, touch);

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

	// start the requested number of threads, less one, and
	// execute an endless loop in each.
	for (i = 1; i < threads; i++)
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
