all: user

user: proj.o readCommands.o tcpandudp.o commands.o validate.o
	gcc -Wall -o user proj.o readCommands.o tcpandudp.o commands.o validate.o

proj.o: proj.c
	gcc -Wall -c proj.c

tcpandudp.o: tcpandudp.c
	gcc -Wall -c tcpandudp.c

readCommands.o: readCommands.c
	gcc -Wall -c readCommands.c

commands.o: commands.c
	gcc -Wall -c commands.c

validate.o: validate.c
	gcc -Wall -c validate.c



