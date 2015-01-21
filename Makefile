all : trace

LC4.o : LC4.h LC4.c
	clang -c LC4.c

ObjectFiles.o : ObjectFiles.h ObjectFiles.c
	clang -c ObjectFiles.c

trace : trace.c LC4.o ObjectFiles.o
	clang -o trace trace.c LC4.o ObjectFiles.o -lm

clean : 
	rm *.o trace
