## software

Generally useful software.

### burn

    // burn s cpu seconds on each of t threads while holding m GB virtual
    // memory of which r% is resident

You build this code with something like

    gcc burn.c -o burn -lpthread

You run it with something like

    burn -t 2 -s 90 -m 8 -r 25

This invocation of burn will consume 100% of each of two threads for ninety
seconds while holding 8 GB of virtual memory, 25% of which is residing in
physical memory.  Invoking

    burn -h

will list the built-in defaults for these values.


### burn_mpi

    //  burn s cpu seconds in t threads while holding m GB virtual memory
    //  of which r% is resident per MPI rank

This code is the MPI version of burn.  You invoke it with something like

    mpirun -n 8 ./burn_mpi -t 2 -s 90 -m 8 -r 25

This will consume 100% of each of each two threads for ninety seconds in
each of 8 MPI ranks, with each rank holding 8 GB of virtual memory, 25%
of which is residing in physical memory.

If your mpirun binds all threads to a single core, use the following instead

    mpirun -n 8 taskset -a -c 0-1023 ./burn_mpi -t 2 -s 90 -m 8 -r 25

Here 1023 should be larger than the number of cores on any host, and
appears to be the current maximum mpirun allows.


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

You run it with something like

    mpirun -np 1024 rapcat R CMD BATCH --vanilla Rtest.R >my_output

... and instead of creating 1,024 output files you create one large one, which contains the
rank outputs in rank order.

### delta

    # show differences between two text files of columnar real numbers
    # with -r, prefix with record number
    # with -s, output only summary lines

You give delta two text files of columnar floating point data, and delta will show you the differences as well as the values with the largest and the next-largest differences.

For example,

% cat t1
1.1 2.3 3.14
1.1 2.31 3.14
1.1 2.3 3.14
% cat t2
1.1 2.3 3.14
1.1 2.3 3.14
1.1 2.3 3.14159
% delta t1 t2
 -  -  -
 -  0.01  -
 -  -  0.00159
7 matches 2 deltas 3 records
max delta 0.01 at record 2 column 2
next largest delta 0.00159 at record 3 column 3
