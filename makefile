OBJS=main.o parse.o scan.o util.o

tiny:$(OBJS)
    $(CC) -o tiny $(OBJS)

main.o:main.c globals.h util.h scan.h util.h
    $(CC) -c main.c

util.o:util.c globals.h util.h
    gcc -c util.o

scan.o:scan.c globals.h util.h scan.h
    gcc -c scan.o

parse.o:parse.c globals.h scan.h parse.h util.h
    gcc -c parse.o

clean:
    rm *.o;rm tiny;