GPP = g++ -O3 -Wall -std=c++17 -pthread -lpthread

A = main2.o

subproject1: $A 
	$(GPP) -o subproject2 main2.o 

main.o: main.cpp
	$(GPP) -c main2.cpp



