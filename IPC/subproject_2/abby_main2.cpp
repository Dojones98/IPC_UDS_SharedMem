#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>
#include <vector>
using std::ifstream;
using std::string;
using std::ref;


void threadFunc(std::vector<string> &v, string word, int filesize_0, int filesize_1 ) {
	
}

void child(int p[2], string word) {
	sem_t *sc = sem_open("s", O_CREAT, 0644, 0);
	/***** GET FILESIZE FROM PARENT *******/
	long int filesize = 0;
	read(p[0], &filesize, sizeof(filesize));
	printf("%ld\n", filesize);

	int filesize_0 = 0;
	int filesize_1 = filesize/4;
	int filesize_2 = filesize/2;
	int filesize_3 = 3*(filesize_1);
	
	/********** DESTINATION **********/
	int fd_shm = shm_open("/shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	ftruncate(fd_shm, filesize);
	char *dest = (char *)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	
	/********* CREATE THREADS **********/
	std::vector<std::string> work_vec; 

	pthread_t t1(threadFunc, std::ref(work_vec), word, filesize_0, filesize_1);
	
	
	printf("before wait\n");
	sem_wait(sc);
	printf("after wait\n");
	
	printf("\n\nChild:\n");
	for(int i = 0; i < filesize; i++) {
		printf("%c",dest[i]);
	}
	printf("\n\nThe word of interest is: %s\n", word.c_str());
	sem_unlink("s");
	
}

void parent(string filepath, int p[2]) {
	sem_unlink("s");
	sem_t *s = sem_open("s", O_CREAT, 0644, 0);
	/********** SOURCE  **********/	
	int fd_file = open(filepath.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
	struct stat sb;
	if(fstat(fd_file, &sb) == -1) {
		perror("couldn't get file size: ");
		exit(1);
	}
	size_t filesize = sb.st_size;
	char *src = (char *)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd_file, 0);
	
	/********* SEND FILESIZE TO CHILD ********/
	write(p[1], &filesize, sizeof(filesize));
	
	shm_unlink("/shmem");
	
	/********** DESTINATION **********/
	int fd_shm = shm_open("/shmem", O_CREAT | O_RDWR, 0660);
	if(ftruncate(fd_shm, filesize) == -1)
		perror("ftruncate");
	char *dest = (char *)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	perror("mmaps");

	memcpy(dest, src, filesize);
	printf("\n\nParent:\n");
	for(int i = 0; i < filesize; i++) {
		printf("%c",dest[i]);
	}
	sem_post(s);
}

int main (int argc, char **argv)
{
    if(argc != 3) {
        printf("Program Must Be Run As: ./program Filepath WordToSearchFor\n");
        exit(1);
    }
    
    /*** CREATE PIPE TO SEND FILESIZE ***/
    int p[2];
    pipe(p);
    
    string word = argv[2];
    
    if(!fork()) {
        child(p, word);
	}
    else {
		parent(argv[1], p);
	}
    
}
