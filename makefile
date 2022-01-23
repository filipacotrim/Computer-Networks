all: 
	cd client; gcc -Wall proj.c readCommands.c tcpandudp.c commands.c validate.c -o ../user
	cd server; gcc -Wall main.c sockets.c readmessages.c commands.c profunctions.c filefunctions.c validate.c -o ../DS

client:
	cd client; gcc -Wall proj.c readCommands.c tcpandudp.c commands.c validate.c -o ../user

server:
	cd server; gcc -Wall main.c sockets.c readmessages.c commands.c profunctions.c filefunctions.c validate.c -o ../DS

clean:
	rm user DS



