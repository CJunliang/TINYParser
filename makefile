OBJS = main.o util.o scan.o parse.o symtab.o analyze.o translate.o

CFLAGS = 

all:$(OBJS)
	$(CC) -o tiny $(OBJS)

main.o: main.c globals.h util.h scan.h parse.h analyze.h translate.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

scan.o: scan.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c scan.c

parse.o: parse.c parse.h scan.h globals.h util.h
	$(CC) $(CFLAGS) -c parse.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c

translate.o: translate.c translate.h globals.h	util.h
	$(CC) $(CFLAGS) -c translate.c

clean:
	-rm main.o
	-rm util.o
	-rm scan.o
	-rm parse.o
	-rm symtab.o
	-rm analyze.o
	-rm translate.o