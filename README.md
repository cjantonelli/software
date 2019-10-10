## software

Generally useful software.

### burn

    //  burn t cpu seconds on c cores while holding m GB virtual memory
    //  of which r% is resident

You build this code with something like

    gcc burn.c -o burn -lpthread

You run it with something like

    burn -c 2 -t 90 -m 8 -r 25

This invocation of burn will consume 100% of each of two cores for ninety
seconds while holding 8 GB of virtual memory, 25% of which is residing in
physical memory.  Invoking

    burn -h

will list the built-in defaults for these values.

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

### logchk

    # check if users have a login on the machine on which logchk is run, and prompt them to
    # get one if not.
    #
    # also checks if they have a SLURM account, and other specific processing.

THis is a specific-purpose script, and will need modification for other applications.

You run the script by preparing an input file of user id's, one per line.  The file is
named something like

    hpc101_20191010.txt

where "hpc101" is the short name for the course, and "20191010" is the date this session
is taught.  The course number is optional, as is the date and its preceding underscore.

To add a course, add a line similar to the following to the course validation case statement:

    mycourse) CLASSNAME="my favorite course"; REASON="you need an account";;

where CLASSNAME is the long name of the course, and REASON is the phrase
justifying why the user is getting the notification email.  Alternatively, the
-m argument can be used to stop the script from checking that the short course
name is known and fron obtaining course name or date taught from the input file
name; logchk will use generic language for the notification.

Use the -d argument to send a single email to yourself instead of the
student, so you can examine the email before sending to everyone.
