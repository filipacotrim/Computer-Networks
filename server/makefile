all: server

server: main.o sockets.o readmessages.o commands.o profunctions.o filefunctions.o validate.o
	gcc -Wall -o server main.o sockets.o readmessages.o commands.o profunctions.o filefunctions.o validate.o

main.o: main.c
	gcc -Wall -c main.c

sockets.o: sockets.c
	gcc -Wall -c sockets.c

readmessages.o: readmessages.c
	gcc -Wall -c readmessages.c

commands.o: commands.c
	gcc -Wall -c commands.c

profunctions.o: profunctions.c
	gcc -Wall -c profunctions.c

filefunctions.o: filefunctions.c
	gcc -Wall -c filefunctions.c

validate.o: validate.c
	gcc -Wall -c validate.c


