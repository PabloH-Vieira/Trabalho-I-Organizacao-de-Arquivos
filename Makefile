all:
	gcc -Wall -g -o programaTrab *.c -lm

run:
	./programaTrab

check: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./programaTrab

# Clean target
clean:
	rm -f *.o programaTrab