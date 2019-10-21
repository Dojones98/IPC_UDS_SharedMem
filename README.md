# IPC_UDS_SharedMem

Exercise using the Unix domain socket and shared memory as forms of Inter-process communication

Author: Daniel Jones

Class: CSCE311

Date: October 21, 2019

Assignment: Project 2

# How to compile and run

## Subproject 1:

1. Launch terminal 
2. Make the directory: "subproject_1", located in the IPC directory the current 
    working directory 
3. Type "make"
4. Run the code by typing "./subproject1"
5. The code will prompt you for a filename, provide either the file to use
    or the relative filepath. There are two text files in the subproject_1 
    directory for you to use, "ANNA_KARENINA.txt" and "big.txt".
6. The code will prompt you for a keyword to search. Provide a keyword. Case 
    does not matter.
7. The program execution will output the lines of the file containing the 
    keyword to standard output.



## Subproject 2:

1. Launch terminal 
2. Make the directory: "subproject_2", located in the IPC directory the current 
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
The child uses another function, "after_search", to create a vector only containing the lines of the 
file containing the keyword. 

After the lines of the file containing the keyword have been selected, the lines are pushed back to 
the parent process using the UDS, one character at a time. The parent process then rebuilds the strings once again, 
pushing them onto a vector and sorting them alphabetically before outputting them via standard output.

## Subproject 2: Shared Memory 

Subproject 2 was designed with the intention of using fork() to create a new process and using Shared Memory as the
method of interprocess communication.

The execution of this program takes in two additional command-line arguments, streamlining the process of beginning
program execution. After this, the parent process creates two pipes, forks, and then the parent calls a function,
"parent()", while the child calls a function "child()." 

The "parent()" function takes in three parameters, a filename and two integer arrays. The two integer arrays
will be used to pipe the size of shared memory from parent to child and child to parent. Pipes were used instead of 
another form of IPC as this made semaphores unncessary as the pipes would wait until they read the value they were expecting. 
The parent function creates a file descriptor using the filename it was passed. It then uses the stat struct to find the size
of the file and creates another file descriptor, but this time with shared memory. Mmap is used to create a block of local 
memory mapped to the file using the first file descriptor created. Mmap is then used again to map a block of memory to the 
open shared memory file descriptor. Memcoy is then used to copy data from the local block of mmap memory to the shared block 
of mmap memory. Parent process then uses one of the pipes to send the size of the file to the child process, so that it knows 
how big the block of allocated memory needs to be.

The child process reads the size of the file in memory from the pipe, p. It then opens a shared memory file descriptor and 
mmaps a section of shared memory to the size of the file sent by the parent process. The child process builds a vector 
of lines of the file by reading from shared memory. Then, that vector is split into 4 parts to be used by the threads
in the "map" part of map-reduce. 

Four threads are created by the child process and each thread takes the reference to one of the vectors and searches for the 
keyword in each line of the vector in the function: "thread_function". A regex is used for string matching. 
Lines that match the string are pushed onto a result vector, and at the end of the string matching, the reference to the 
vector passed to the function is set to the reult vector.

The child process then calls "join" for all of the threads and combines all four sub-vectors into one main vector,
for the "reduce" portion of map-reduce. After this, the child initializes a variable called tracker to track how big the 
block of memory needs to be to store the result vector, while it does this, the result vector is being put back into the 
shared memory. The child then writes the size of the resultant vector to the parent process via the second pipe so that 
the parent knows how much shared memory to access.

Finally, the parent process reads the size of the resultant vector from the second pipe passed to the parent and child.
It then uses a for loop to rebuild the characters in memory into a vector of strings for each line in the original file.
The parent sorts the vector of strings and then outputs it to standard output and unlinks the segment of shared memory.



