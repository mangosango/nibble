CC = gcc
OBJS = crawler.o html.o hash.o
CFLAGS = -Wall -pedantic -std=c99
EXEC = crawler
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< header.h
clean:
	rm -f *~
	rm -f *#
	rm -f ./prs
	rm -f *.o
	rm -rf ./searchData/*
