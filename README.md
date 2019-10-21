# IPC_UDS_SharedMem
Exercise using the Unix domain socket and shared memory as forms of Inter-process communication

Code designed and written by Daniel Jones 
CSCE 311, Project 

# How to compile and run

## Subproject 1:

1. Launch terminal 
2. Make the directory "subproject_1" located in the IPC directory the current 
    working directory 
3. Type "make"
4. Run the code by typing "./subproject1"
5. The code will prompt you for a filename, provide the either the file to use
    or the relative filepath. There are two text files in the subproject_1 
    directory for you to use, "ANNA_KARENINA.txt" and "big.txt".
6. The code will prompt you for a keyword to search, provide a keyword. Case 
    does not matter.
7. The program execution will output the lines of the file containing the 
    keyword to standard output.



## Subproject 2:

1. Launch terminal 
2. Make the directory "subproject_2" located in the IPC directory the current 
    working directory 
3. Type "make"            
4. This program requires two additional command line arguments, so you will 
    need to execute the program with the following command:
    "./subproject_2 <filename> <keyword>"
6. The program execution will output the lines of the file containing the 
    keyword to standard output.   


# Program Design:

## Subproject 1: Unix Domain Socket

Subproject 1 was designed with the intention of using fork() to create a new process and 
using the Unix Doman Socket as the method of inter-process communication. 

The program takes two values from standard input, the name of the file, and the keyword to 
be searched. The parent process then forks. Parent process then opens the file and passes it
to the child using the UDS and a character buffer of size 1. One character at a time, the child 
process rebuilds the strings of the file into its lines and pushes them back to a vector. After this,
The child uses another function, after_search, to create a vector only containing the lines of the 
file using the keyword. 

After the lines of the file containing the keyword have been selected, the lines are pushed back to 
the parent process using the UDS. The parent process then rebuilds the strings once again, pushing them
onto a vector and sorting them alphabetically before outputting them via standard output.

## Subproject 2: Shared Memory 

Subproject 2 was designed with the intention of using fork() to create a new process and using Shared Memory as the method interprocess communication.

The execution of this program takes in two additional commandline arguments, streamlining the process of beginning
program execution. After this the parent process creates two pipes, forks, and then the parent calls a function,
"parent()", while the child calls a function "child()." 

The "parent()" function takes in three parameters, a filename, an int array, and another int array. The two int arrays
will be used to pipe the size of shared memory from parent to child and child to parent. Pipes were used instead of 
another form of IPC as this made semaphores unncessary as the pipes would wait until they read the value they were expecting. 
The parent function creates a file descriptor using the filename it was passed. It then uses the stat struct to find the size
of the file and creates another file descriptor, but this time with shared memory. Mmap is used to create a block of local 
memory mapping the file, using the first file descriptor created. Mmap is then used again to map a block of memory to the 
open shared memory file descriptor. Memcoy is then used to copy data from the local block of mmap memory to the shared block 
of mmap memory. Parent process then uses one of the pipes to send the size of the file to the child process, so that it knows 
how big the block of allocated memory needs to be.



