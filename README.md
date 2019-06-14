## software

Generally useful software.

### rapcat

// written to replace array job with mpi rank execution of R code
//
// each rank executes the R command specified on the command line with PBS_ARRAY_ID set.
// rank 0 gatherv's all ranks' standard output onto its standard output.
//
// this program can be used to execute arbitrary commands.  if the specified command
// is an R command (that is, starts with "R ") the string /dev/fd/1 is appended to
// force the R command's output to stdout.  this will fail if the R command specifies
// its own file to be used for the command's output, which it shouldn't be doing anyway.

This code uses the MPI library and environment.  You build this code with something like

    mpicc -o rapcat rapcat.c

You run it wuth something like

    mpirun -np 1024 rapcat R CMD BATCH --vanilla Rtest.R >my_output

... and instead of creating 1,024 output files you create one large one, which contains the
rank outputs in rank order.
