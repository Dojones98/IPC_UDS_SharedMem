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
#include <regex>
#include <thread>

using std::regex;
using std::smatch;
using std::regex_search;
using std::regex_constants::icase;
using std::ifstream;
using std::string;
using std::ref;
using std::vector;
using std::thread;


void *threadFunc(vector<string> &v) {
	vector<string> matched;
	string word = v.back();  // Get the key word from the vectors and remove it
	v.pop_back();
	
	/**** SEARCH FOR THE KEY WORD *****/
	regex key("\\b"+word+"\\b", icase);  // set the word of interest to a regex
	smatch match; 
	for(int i = 0; i < v.size(); i++) {
		if(regex_search(v.at(i), match, key))
			matched.push_back(v.at(i));
	}
	v = matched;
	pthread_exit(NULL);
}

void child(int p1[2], int p2[2], string word) {
	sem_t *sc = sem_open("s", O_CREAT, 0644, 0);
	
	/***** GET FILESIZE FROM PARENT *******/
	long int filesize = 0;
	read(p1[0], &filesize, sizeof(filesize));
	
	/********** DESTINATION **********/
	int fd_shm = shm_open("/shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	ftruncate(fd_shm, filesize);
	char *dest = (char *)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	sem_wait(sc);
	
	/******** CREATE VECTOR OF EACH LINE *********/
	string line = "";
	vector<string> v;
	char *test = dest;
	for (int i = 0; i < filesize; i++) {
		line+=dest[i];
		if(dest[i] == '\n') {
			v.push_back(line);
			line = "";
		}
	}
	
	/********** SPLIT INTO 4 VECTORS **********/
	vector<string> v1;
	vector<string> v2;
	vector<string> v3;
	vector<string> v4;
	
	for(int i = 0; i < v.size(); i++) {
		if(i%4 ==0)
			v1.push_back(v[i]);
		if(i%4 ==1)
			v2.push_back(v[i]);
		if(i%4 ==2)
			v3.push_back(v[i]);
		if(i%4 ==3)
			v4.push_back(v[i]);
	}
	
	/***** PUSH WORD ONTO VECTOR *****/
	v1.push_back(word);
	v2.push_back(word);
	v3.push_back(word);
	v4.push_back(word);
	
	/*********  CREATE THREADS *********/
	
	thread t1(threadFunc, ref(v1));
	thread t2(threadFunc, ref(v2));
	thread t3(threadFunc, ref(v3));
	thread t4(threadFunc, ref(v4));
	
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	
	/****** COMBINE MATCHED VECTORS INTO V1 *****/
	v1.insert(v1.end(), v2.begin(), v2.end());
	v3.insert(v3.end(), v4.begin(), v4.end());
	v1.insert(v1.end(), v3.begin(), v3.end());
	
	long long int counter = 0;
	for (int i = 0; i < v1.size(); i++) {
		for (int j = 0; j < v1.at(i).length(); j++) {
			dest[counter] = v1.at(i).at(j);
			counter++;
		}
	}
	
	/********* SEND FILESIZE TO PARENT ********/
	write(p2[1], &counter, sizeof(counter));	
	sem_unlink("s");	
}

void parent(string filepath, int p1[2], int p2[2]) {
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
	write(p1[1], &filesize, sizeof(filesize));
	
	if (shm_unlink("/shmem") != 0) {
		perror("unlink");
		exit(1);
	}
	
	/********** DESTINATION **********/
	int fd_shm = shm_open("/shmem", O_CREAT | O_RDWR, 0660);
	if(ftruncate(fd_shm, filesize) == -1)
		perror("ftruncate");
	char *dest = (char *)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	memcpy(dest, src, filesize);
	sem_post(s);
	
	/***** GET FILESIZE FROM CHILD *******/
	long int counter = 0;
	read(p2[0], &counter, sizeof(counter));
	
	/*** GET THE MATCHED LINES ***/
	vector<string> matched_lines;
	string line = "";
	for (int i = 0; i < counter; i++) {
		line+=dest[i];
		if(dest[i] == '\n') {
			matched_lines.push_back(line);
			line = "";
		}
	}
	
	/*** SORT MATCHED LINES AND PRNT THEM OUT ***/
	sort(matched_lines.begin(), matched_lines.end());
	for (int i = 0; i < matched_lines.size(); i++) {
		printf("%s",matched_lines.at(i).c_str());  // print the output of the sorted lines containing the word of interest
	}
}

int main (int argc, char **argv)
{
    if(argc != 3) {
        printf("Program Must Be Run As: ./program Filepath WordToSearchFor\n");
        exit(1);
    }
    
    /*** CREATE PIPE TO SEND FILESIZE TO CHILD ***/
    int p1[2];
    pipe(p1);
    
    /*** CREATE PIPE TO SEND FILESIZE BACK TO PARENT***/
    int p2[2];
    pipe(p2);
    
    string word = argv[2];
    
    if(!fork()) {
        child(p1, p2, word);
	}
    else {
		parent(argv[1], p1, p2);
	}
    
}
