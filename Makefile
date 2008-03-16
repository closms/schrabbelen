CFLAGS = -g

scrabble_helper:  main.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) main.o scrabble_helper
