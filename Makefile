
all: shell server

server: server.c
	gcc -o server server.c

shell: shell.c
	gcc -o shell shell.c

%.o: %.c
	gcc --compile $< -o $@

clean:
	rm -f *.o shell server