#targets: critbit.pdf critbit.o
targets: critbit.o critbit-test

critbit.pdf: critbit.w
	cweave critbit.w
	pdftex critbit.tex

#critbit.c: critbit.w
#	ctangle critbit.w

#ctangle critbit.w
critbit.o: critbit.c
	gcc -Wall -O3 -std=c99 -c critbit.c

critbit-test: critbit.o critbit-test.c
	gcc -Wall -O3 -std=c99 -c critbit-test.c
	gcc -o critbit-test critbit.o critbit-test.o

clean:
	rm -f *.o critbit-test
