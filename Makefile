targets: critbit.pdf critbit.o

critbit.pdf: critbit.w
	cweave critbit.w
	pdftex critbit.tex

critbit.c: critbit.w
	ctangle critbit.w

critbit.o: critbit.c
	ctangle critbit.w
	gcc -Wall -c critbit.c -std=c99 -ggdb
