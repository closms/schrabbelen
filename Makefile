CFLAGS = -W -Wall -ggdb

PRG = scrabhlp

OBJS = main.o hsearch.o hsearch_r.o

$(PRG):  $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(PRG)
