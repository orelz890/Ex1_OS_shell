
all: shell server

server: server.c
	gcc -o server server.c

shell: main.c
	gcc -o shell main.c

%.o: %.c
	gcc --compile $< -o $@

clean:
	rm -f *.o shell server