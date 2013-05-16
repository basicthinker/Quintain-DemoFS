all : 
	gcc -c -o easyzlib.o easyzlib.c
	g++ -std=c++0x -o analyser.o analyser.cpp easyzlib.o -lcrypto

clean :
	rm easyzlib.o analyser.o
