// rapid catenation of small output files
//
// framework stolen from c_ex08.c
//
// written to replace array job with mpi rank execution of R code
//
// each rank executes the R command specified on the command line with PBS_ARRAY_ID set.
// rank 0 gatherv's all ranks' standard output onto its standard output.
//
// this program can be used to execute arbitrary commands.  if the specified command
// is an R command (that is, starts with "R ") the string /dev/fd/1 is appended to
// force the R command's output to stdout.  this will fail if the R command specifies
// its own file to be used for the command's output, which it shouldn't be doing anyway.
//
// cja 14 jun 19

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <libgen.h>

int numnodes,myid,mpi_err;
#define mpi_root 0

int main(int argc, char **argv)
{
	int *displacements,*sizes;
	char *allray;
	int size,mysize,i;
	char *myray;
	int myraysize = 1024;
	FILE *pfd;
	int cmdsize;
	char *cmdbuf, *suffix = "/dev/fd/1";
	char lilbuf[16];
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numnodes );
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	// determine length of command to be executed
	if (myid == mpi_root)
		if (argc < 2) {
			fprintf(stderr, "Usage: %s R-command\n", basename(argv[0]));
			exit(1);
		}
	else {
		cmdsize = strlen(suffix);
		for (i = 1; i < argc; i++)
			cmdsize += strlen(argv[i])+1;
	}

	// construct the command and send to all workers
	MPI_Bcast(&cmdsize, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if ((cmdbuf = malloc(cmdsize*sizeof(char))) < 0)
		err(1, "cmdbuf malloc");

	if (myid == mpi_root) {
		cmdbuf[0] = '\0';
		for (i = 1; i < argc; i++) {
			strcat(cmdbuf, argv[i]);
			strcat(cmdbuf, " ");
		}
		// if we're running R, force output to stdout
		if (cmdbuf[0] == 'R' && cmdbuf[1] == ' ')
			strcat(cmdbuf, suffix);
	}

	MPI_Bcast(cmdbuf, cmdsize, MPI_CHAR, 0, MPI_COMM_WORLD);

	// execute command and collect its standfard output
fprintf(stderr, "%d: &cmdbuf=%p cmdsize=%d cmdbuf=%s\n", myid, cmdbuf, cmdsize, cmdbuf);
	sprintf(lilbuf,"%d", myid+1);
	if (setenv("PBS_ARRAY_ID", lilbuf, 1) < 0)
		err(1, "setenv");
	pfd = popen(cmdbuf, "r");
	if ((myray = malloc(myraysize*sizeof(char))) < 0)
		err(1, "myray malloc");
	int n = 0, tn = 0;
	char *p = myray;
	for (myray[0] = '\0'; !feof(pfd);) {
		n = fread(p, 1, myraysize-tn-1, pfd);
		p += n;
		tn += n;
		*p = '\0';
		if (p >= myray+myraysize-1) {
			int diff = p - myray;
			if ((myray = realloc(myray, myraysize *= 2)) < 0)
				err(1, "myray realloc");
			p = myray + diff;
		}
	}
	pclose(pfd);

	mysize=strlen(myray); // no null at end of each chunk

	/* sizes and displacement arrays are only required on the root */
	if(myid == mpi_root){
		sizes=(int*)malloc(numnodes*sizeof(int));
		displacements=(int*)malloc(numnodes*sizeof(int));
	}

	/* gather the sizes to the root */
	MPI_Gather((void*)&mysize, 1, MPI_INT, (void*)sizes, 1, MPI_INT, mpi_root, MPI_COMM_WORLD);

	/* calculate displacements and the size of the recv array */
	if(myid == mpi_root){
		displacements[0]=0;
		for( i=1;i<numnodes;i++){
			displacements[i]=sizes[i-1]+displacements[i-1];
		}
		size=0;
		for(i=0;i< numnodes;i++)
			size=size+sizes[i];
		if ((allray=(char *)malloc((size+1)*sizeof(char))) < 0)
				err(1, "allray malloc");
	}

	/* gather different amounts of data from each processor to the root */
	MPI_Gatherv(myray, mysize, MPI_CHAR, allray, sizes, displacements, MPI_CHAR, mpi_root, MPI_COMM_WORLD);
	                
	/* output all the data */
	if(myid == mpi_root){
		allray[size] = '\0'; // one null at end of concatenated chunks
		fputs(allray, stdout);
	}

	MPI_Finalize();
}
