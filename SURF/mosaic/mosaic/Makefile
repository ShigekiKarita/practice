all:	mosaic

mosaic:	main.c pnm.c mosaic.c pnm.h mosaic.h Makefile
	gcc -g -Wall -O3 main.c pnm.c mosaic.c -o mosaic -lm

clean:	
	/bin/rm -f mosaic compare *.o *~ core gmon.out
