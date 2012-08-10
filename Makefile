#targets: critbit.pdf critbit.o
targets: critbit.o critbit-test

critbit.pdf: critbit.w
	cweave critbit.w
	pdftex critbit.tex

#critbit.c: critbit.w
#	ctangle critbit.w

#ctangle critbit.w
critbit.o: critbit.c
	gcc -Wall -c critbit.c -std=c99 -ggdb

critbit-test: critbit.o critbit-test.c
	gcc -std=c99 -Wall -c critbit-test.c -ggdb
	gcc -o critbit-test critbit.o critbit-test.o

clean:
	rm *.o critbit-test
