//atualizado
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>


/**000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

verificar sempre as cenas que vem das funcoes antes de mandar a mensagem para nao mandar a mensaem anterior


000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
*/

#define DEFAULT_DSPORT "58061"   //default port if not specified 58000+Gnumber
#define DEFAULT_DSIP "127.0.0.1" // localhost
#define IP_SIZE 64
#define PORT_SIZE 16
#define UID_SIZE 5
#define PASS_SIZE 8
#define OPCODE_SIZE 4
#define MAX_GID 2
#define LOGGED_IN 1
#define LOGGED_OUT 0
#define ACTIVATED 1
#define DEACTIVATED 0
#define NON_ACTIVE 0
#define BUFFER_SIZE 240
#define MAX_TOKENS_COMMAND 4
#define MAX_TOKENS_RES 300
#define GNAME_SIZE 24
#define CAN_SEND 1
#define Tsize 242
#define MAX_FNAME 24

int fdDSUDP, fdDSTCP, errcode;
ssize_t nUDP;
ssize_t nTCP;
socklen_t addrlenUDP;
socklen_t addrlenTCP;
struct addrinfo hintsDSTCP, *resDSTCP;
struct addrinfo hintsDSUDP, *resDSUDP;
struct sockaddr_in addrDSTCP;
struct sockaddr_in addrDSUDP;


char portDS[PORT_SIZE] = DEFAULT_DSPORT;
char ipDS[IP_SIZE] = DEFAULT_DSIP;
char buffer[BUFFER_SIZE]; // so para ler do terminal o comando
char uid[UID_SIZE] = ""; // saber o uid do homem
char pass[PASS_SIZE] = ""; // saber a pass do homem
char opcode[OPCODE_SIZE] = ""; // saber o opcode do comando
char message[BUFFER_SIZE] = ""; // saber a mensagem que vamos enviar
int session = LOGGED_OUT; // saber se o homem ja esta logIN ou nao 
int activeGid = DEACTIVATED;
char gid[MAX_GID] = ""; // id do grupo 
char active_gid[MAX_GID] =""; // grupo ativo 
char gname[GNAME_SIZE] = ""; // nome do grupo 
char text[Tsize] = ""; //tamanho da mensagem
char Fname[] = ""; //tamanho file name



void checkingInput(int argc, char *argv[]); // VALIDAR O INPUT
int readCommands(); // read commands given by the user
void sendMessage(); // sending the message through tcp or udp
void errorMsg(char *errMsg); // error mensage
int registerCommand(char *token_list[], int num_tokens); // command register
int unregisterCommand(char *token_list[], int num_tokens); // command unregister
int loginCommand(char *token_list[], int num_tokens); // command login
int logoutCommand(char *token_list[], int num_tokens);
int subscribeCommand(char *token_list[], int num_tokens); // command subscribe
int unsubscribeCommand(char *token_list[]); // unsubscribe command
int showuidCommand(char *token_list[], int num_tokens); // showuid command
int my_groupsCommand(); //my_gorups command
int selectCommand(char *token_list[]); // select command
int listCommand();
int postCommand(char *token_list[],int num_tokens); // select command
int retrieveCommand(char *token_list[], int num_tokens); // retrieve command
void sendMessageUDP(char *message);
void sendMessageTCP(char *message);
int isNumber(char* s); // checking if the str is a number
int isAlpha(char* str, int flag); // checking if the str is alphanumeric
void status(char uid[], char pass[], char message[], int session); // check the status




/** 000000000000000000000000000

 	Main function

000000000000000000000000000000*/
int main(int argc, char *argv[]){
	checkingInput(argc, argv);

	// inicializar UDP
	fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0); // UDP SOCKET
	if(fdDSUDP == -1) exit(1);

	memset(&hintsDSUDP, 0, sizeof hintsDSUDP);
	hintsDSUDP.ai_family = AF_INET;
	hintsDSUDP.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo(ipDS, portDS, &hintsDSUDP, &resDSUDP);
	if(errcode != 0) {errorMsg("Error while getting address info.\n");exit(1);}

	// MAIN PROGRAM
	readCommands();
	exit(EXIT_SUCCESS);
}






/**
	reads the commands given by the user
*/
int readCommands(){
	while(1){
		memset(opcode, 0, OPCODE_SIZE);
		memset(buffer, 0, BUFFER_SIZE);
		memset(message, 0, BUFFER_SIZE);

		//read user's input
		printf("Enter a command: ");
		fgets(buffer, BUFFER_SIZE, stdin);
		
		int num_tokens = 0;
		char* token_list[MAX_TOKENS_COMMAND]; 
		char* token = strtok(buffer, " \n");
		
		// KNOW THE TOKENS WRITTEN 
		while (token != NULL && num_tokens < MAX_TOKENS_COMMAND){
			token_list[num_tokens++] = token;
			token = strtok(NULL, " \n");
		} 

	
		
		// R E G I S T E R
		if(!strcmp(token_list[0], "reg") && num_tokens == 3){ // 3 tokens -> reg uid pass
			if(registerCommand(token_list, num_tokens))
				sendMessage(message, "UDP");
			continue;
		}

		// U N R E G I S T E R
		else if((!strcmp(token_list[0], "unregister") || !strcmp(token_list[0], "unr")) && num_tokens == 3){
			if(unregisterCommand(token_list, num_tokens))
				sendMessage(message, "UDP");
			continue;
		}

		// L O G I N
		else if(!strcmp(token_list[0], "login") && num_tokens == 3){ // 3 tokens -> LOG UID PASS 
			if(loginCommand(token_list, num_tokens))
				sendMessage(message, "UDP");
			continue;
		}

		// L O G O U T (this command is performed locally)
		else if(!strcmp(token_list[0], "logout") && num_tokens == 1){
			if(logoutCommand(token_list, num_tokens))
				sendMessage(message, "UDP");
			continue;
		}

		// S H O W U I D (this command is performed locally)
		else if((!strcmp(token_list[0], "showuid") || !strcmp(token_list[0], "su")) && num_tokens == 1){
			showuidCommand(token_list, num_tokens);
			continue;
		}

		// G R O U P S
		else if((!strcmp(token_list[0],"groups") || !strcmp(token_list[0],"gl")) && num_tokens == 1){
			sprintf(message, "GLS\n");
			sendMessage(message, "UDP");
			continue;
		}

		// S U B S C R I B E
		else if(!strcmp(token_list[0], "subscribe") && num_tokens == 3){
			if(subscribeCommand(token_list, num_tokens))
				sendMessage(message, "UDP");
			continue;
		}

		// U N S U B S C R I B E
		else if((!strcmp(token_list[0],"unsubscribe") || !strcmp(token_list[0],"u")) && num_tokens == 2){
			if(unsubscribeCommand(token_list))
				sendMessage(message, "UDP");
			continue;
		}

		// M Y  G R O U P S
		else if((!strcmp(token_list[0],"my_groups") || !strcmp(token_list[0],"mgl")) && num_tokens == 1){
			if(my_groupsCommand())
				sendMessage(message,"UDP");
			continue;
		}

		// S E L E C T 
		else if((!strcmp(token_list[0],"select") || !strcmp(token_list[0],"sag")) && num_tokens == 2){
			selectCommand(token_list);
			continue;
		}

		// S H O W G I D
		else if((!strcmp(token_list[0],"showgid") || !strcmp(token_list[0],"sg")) && num_tokens == 1) {
			//user application locally displays the GID of the selected group???
			printf("GID of the selected group: %s\n", active_gid);
			continue;
		}

		// U L I S T
		else if((!strcmp(token_list[0], "ulist") || !strcmp(token_list[0], "ul") && num_tokens == 1)){
			if(listCommand()){
				puts("Antes do Send Message");
				sendMessageTCP(message);
				puts("depois do send Message");
			}	
			continue;	
		}

		// P O S T
		else if(!strcmp(token_list[0], "post") && num_tokens == 3){ // 3 tokens -> post "text" [FName] 
			if(postCommand(token_list, num_tokens))
				sendMessage(message, "TCP");
			continue;
		}


		// R E T R I E V E
		else if(!strcmp(token_list[0], "retrieve") || !strcmp(token_list[0], "r") && num_tokens == 2){
			if(retrieveCommand(token_list, num_tokens))
				sendMessage(message, "TCP");
			continue;
		}
		
		
		// D E F A U L T 
		else{
			printf("Invalid command.\n"); 
			continue;
		} 	
	}

	freeaddrinfo(resDSUDP);
	close(fdDSUDP);

    return 0;
}



/**000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

				commands

0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000*/

/**
	registers a new id

	input:
		+ list of parameters read in the input 
		+ number of parameters read in the input
	*/
int registerCommand(char *token_list[], int num_tokens){
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		printf("Invalid UID: %s. Must have 5 numeric characters.\n", token_list[1]);
		return 0; // error
	}
	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		printf("Invalid password: %s. Must have 8 characters and contain only alphanumerical characters.\n", token_list[2]);
		return 0;
	}

    else{ // check uid and password
		strcpy(uid, token_list[1]); 
		strcpy(pass, token_list[2]);
	} 

	sprintf(message, "REG %s %s\n", uid, pass);
	return 1;
}



/**
	unregisters a new id

	input:
		+ list of parameters read in the input
		+ number of parameters read in the input
	*/
int unregisterCommand(char *token_list[], int num_tokens){
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		printf("Invalid UID: %s. Must have 5 numeric characters.\n", token_list[1]);
		return 0;
	}
			
	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		printf("Invalid password: %s. Must have 8 characters and contain only alphanumerical characters.\n", token_list[2]);
		return 0;
	}

	// checking if user is a number and pass is alphanumeric
	else { 
		strcpy(uid, token_list[1]);
		strcpy(pass, token_list[2]);
	} 

	sprintf(message, "UNR %s %s\n", uid, pass);
	return 1;
}




/**
	login of a user

	input:
		+ list of parameters read in the input
		+ number of parameters read in the input
	*/
int loginCommand(char *token_list[], int num_tokens){
	if(session == LOGGED_IN){
		printf("Already logged in as UID %s\n", uid);
		return 0;
	}
			
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		printf("Invalid UID: %s. Must have 5 numeric characters.\n", token_list[1]);
		return 0;
	}

	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		printf("Invalid password: %s. Must have 8 characters and contain only alphanumerical characters.\n", token_list[2]);
		return 0;
	}
	
	// checking if uid is a number and pass is alphanumeric
	else {
		strcpy(uid, token_list[1]);
		strcpy(pass, token_list[2]);
	} 

	sprintf(message, "LOG %s %s\n", uid, pass);
	return 1;
}



/**
	logouts a user

	input:
		+ list of parameters read in the input 
		+ number of parameters read in the input
	*/
int logoutCommand(char *token_list[], int num_tokens){
	if( session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}

	sprintf(message, "OUT %s %s\n", uid, pass);
	return 1;
}

/**
 * show's the id of the user logged in
 * 
 * input:
 *	 + list of parameters read in the input 
 *	 + number of parameters read in the input
 */
int showuidCommand(char *token_list[], int num_tokens){
	if(session == LOGGED_OUT){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	printf("UID: %s\n", uid);
	return 1;
}


/**
 * subscribe command
 * 
 * input:
 *  + token_list
 *  + num_tokens
 * 
*/
int subscribeCommand(char *token_list[], int num_tokens){
	if( session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	//check gid
	if(strlen(token_list[1]) > MAX_GID || !isNumber(token_list[1])){ //GID must be a 2-digit number
		printf("Invalid GID: %s. Must be 2-digit number.\n", token_list[1]);
		return 0;
	}

	//set GID
	strcpy(gid, token_list[1]); 
				
	//check gname
	if(strlen(token_list[2]) > GNAME_SIZE || !isAlpha(token_list[2],1)){ //gname must have 24 alphanumerical characters
		printf("Invalid GName: %s. Is limited to a total of 24 alphanumerical characters.\n", token_list[2]);
		return 0;
	}
	strcpy(gname, token_list[2]);
			
	sprintf(message, "GSR %s %s %s\n", uid, gid, gname);
	return 1;
}

/**
 * unsubscribe command
 * 
 * input:
 * 	+ token_list
 *
*/
int unsubscribeCommand(char *token_list[]){
	if( session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	//check gid
	if(strlen(token_list[1]) > MAX_GID || !isNumber(token_list[1])){ //GID must be a 2-digit number
		printf("Invalid GID: %s. Must be 2-digit number.\n", token_list[1]);
		return 0;
	}

	//set GID
	strcpy(gid, token_list[1]); 

	sprintf(message, "GUR %s %s\n", uid, gid);
	return 1;
}

/**
 * mgl command
 * 
 * input:
 *  + token list	
*/
int my_groupsCommand(){
	if(session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}

	sprintf(message, "GLM %s\n", uid);
	return 1;
}


/**
 * select command
 * 
 * input:
 *  + token list	
*/
int selectCommand(char *token_list[]){
	if(session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	//check gid
	if(strlen(token_list[1]) > MAX_GID || !isNumber(token_list[1])){ //GID must be a 2-digit number
		printf("Invalid GID: %s. Must be 2-digit number.\n", token_list[1]);
		return 0;
	}

	//set active group GID
	strcpy(active_gid, token_list[1]);
	printf("Group %s selected.\n", active_gid);
	activeGid = ACTIVATED;
	return 1;
}


/**
 * ulist command
 * 
 */
int listCommand(){
	if(session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	if(activeGid == DEACTIVATED){
		printf("You must have a group selected to perform this action first.\n");
		return 0;
	}
	puts("Antes de criar mensagem");
	sprintf(message, "ULS %s\n", active_gid);
	return 1;
}

int postCommand(char *token_list[],int num_tokens) {
	if(session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	if(activeGid == DEACTIVATED){
		printf("You must have a group selected to perform this action first.\n");
		return 0;
	}
	return 1;
	//verificar aspas
	//if(strlen(token_list[1]) > Tsize || !isFile(token_list[2]) || !quotation)
	
	
}



/**
 * retreive command
 * 
 * input: 
 * 	+ token_list
 *  + num_tokens
 */
int retrieveCommand(char *token_list[], int num_tokens){
	// if there's no session logged in 
	if(session == LOGGED_OUT){
		printf("You must login to perform this action first. \n");
		return 0;
	}

	// there's no group active at the moment
	if(activeGid == DEACTIVATED){
		printf("You must have a group selected to perform this action first.\n");
		return 0;
	}
	if(strlen(token_list[1]) > 4 || !isNumber(token_list[1])) {
		printf("Invalid MID\n");
		return 0;
	}
	
	sprintf(message, "RTV %s %s %s", uid, active_gid, token_list[1]);
	return 1;

}


/**000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

				sending message (udp & tcp)
				
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000*/

/**
 * process the message given by the server in TCP protocol
 * 
 * input:
 * 	+ message from the server
 */
void processResponseTCP(char *msg){
	int num_tokens = 0;
	int counter = 0;
	char* token_list1[MAX_TOKENS_RES]; 
	char* token = strtok(msg, " \n");
	
	// KNOW THE TOKENS WRITTEN 
	while (token != NULL && num_tokens < MAX_TOKENS_RES){
		token_list1[num_tokens++] = token;
		token = strtok(NULL, " \n");
		counter++;
	}
	
	if(!strcmp(token_list1[0], "RUL")){ // ULIST
		if(!strcmp(token_list1[1], "OK")){
			printf("Users in group %s:\n", token_list1[2]);
			for(int k = 3; k < counter; k++){
				printf("UID: %s\n", token_list1[k]);
			}
		} else if(!strcmp(token_list1[1], "NOK"))
			//group does not exist
			printf("This group does not exist.\n");
	}
	else if(!strcmp(token_list1[0], "RTV")){
		if(!strcmp(token_list1[1], "OK")) {
			//printf("");
		}
	}
}


/**
 * process the message given by the server in UDP messages 
 * 
 * input:
 * 	+ message from the server
*/
void processResponseUDP(char *message){
	int num_tokens = 0;
	int counter = 0;
	char* token_list[MAX_TOKENS_RES]; 
	char* token = strtok(message, " \n");
		
	// KNOW THE TOKENS WRITTEN 
	while (token != NULL && num_tokens < MAX_TOKENS_RES){
		token_list[num_tokens++] = token;
		token = strtok(NULL, " \n");
		counter++;
	}
	
	if(!strcmp(token_list[0], "RRG")){ // REGISTER
		//registration successfull
		if(!strcmp(token_list[1], "OK"))
			printf("Successfully registered user.\n");
		//user already exists
		else if(!strcmp(token_list[1], "DUP"))
			printf("This ID is already belongs to another user.\n");
		//too many users registered
		else if(!strcmp(token_list[1], "NOK"))
			printf("Registration failed. Too many users registered already.\n");
		
	} else if(!strcmp(token_list[0], "RUN")){ // UNREGISTER 
		if(!strcmp(token_list[1], "OK"))
			//unregistration successfull
			printf("Successfully unregistered user.\n");
		else if(!strcmp(token_list[1], "NOK"))
			//invalid UID or incorrect pass
			printf("Invalid UID or incorrect password. Failed to unregistered.\n");
	
	} else if(!strcmp(token_list[0], "RLO")){ // LOGIN
		if(!strcmp(token_list[1], "OK")){
			//login successfull
			printf("Successfully logged in.\n");
			session = LOGGED_IN;
		} else if(!strcmp(token_list[1], "NOK"))
			//invalid UID or incorrect pass
			printf("Invalid UID or incorrect password. Failed to log in.\n");

	} else if(!strcmp(token_list[0], "ROU")){ // LOGOUT
		if(!strcmp(token_list[1], "OK")){
			//logout successfull
			printf("Successfully logged out.\n");
			session = LOGGED_OUT;
		} else if(!strcmp(token_list[1], "NOK"))
			//invalid UID or incorrect pass
			printf("Invalid UID or incorrect password. Failed to log out.\n");

	} else if(!strcmp(token_list[0], "RGL")){ // GROUPS
		printf("There are currently %s groups.\n", token_list[1]);
		for(int k = 2; k < counter; k=k+3){
        	printf("GID: %s/ GName: %s/ MID: %s\n", token_list[k], token_list[k+1], token_list[k+2]);
    	}
	} else if(!strcmp(token_list[0], "RGS")){ // SUBSCRIBE
		if(!strcmp(token_list[1], "OK"))
			//ok	
			printf("Successfully subscribed to group %s.\n", gname);
		else if(!strcmp(token_list[1], "NEW"))
			//create new group
			printf("Successfully created and subscribed group %s.\n", gname);
		else if(!strcmp(token_list[1], "E_USR"))
			//invalid UID
			printf("Invalid user ID.\n");
		else if(!strcmp(token_list[1], "E_GRP"))
			//invalid GID
			printf("Invalid group ID.\n");
		else if(!strcmp(token_list[1], "E_GNAME"))
			//invalid GNAME
			printf("Invalid group name.\n");
		else if(!strcmp(token_list[1], "E_FULL"))
			//max reached
			printf("Error. Maximum number of groups has already been reached.\n");
		else if(!strcmp(token_list[1], "NOK"))
			//other error
			printf("Failed to subscribe/create group.\n");


	} else if(!strcmp(token_list[0], "RGU")){ // UNSUBSCRIBE
		if(!strcmp(token_list[1], "OK"))
			//ok	
			printf("Successfully unsubscribed from group.\n");
		else if(!strcmp(token_list[1], "E_USR"))
			//invalid UID
			printf("Invalid user ID.\n");
		else if(!strcmp(token_list[1], "E_GRP"))
			//invalid GID
			printf("Invalid group ID.\n");
		else if(!strcmp(token_list[1], "NOK"))
			//other error
			printf("Error. Failed to unsubscribe from group.\n");

	} else if(!strcmp(token_list[0], "RGM")){ // MY_GROUPS
		printf("You are currently subscribed to %s groups.\n", token_list[1]);
		for(int k = 2; k < counter; k=k+3){
        	printf("GID: %s/ GName: %s/ MID: %s\n", token_list[k], token_list[k+1], token_list[k+2]);
    	}
	}
}


/**
 * sends message to the server
 * 
 * input:
 * 	+ message to the server
 *  + protocol to follow
*/
void sendMessage(char *message, char* protocol){
	if(!strcmp(protocol, "UDP")){sendMessageUDP(message);} // fazer de forma PERSISTENT
	else if(!strcmp(protocol, "TCP")){sendMessageTCP(message);}
}


/**
 * send message in UDP protocol
 * 
 * input:
 * 	+ message to the server
*/
void sendMessageUDP(char *message){
	char bufferUDP[5000];
	// sending the message
	nUDP = sendto(fdDSUDP, message, strlen(message), 0, resDSUDP->ai_addr, resDSUDP->ai_addrlen);
	if(nUDP == -1) exit(1);

	// receiving the message
	addrlenUDP = sizeof(addrDSUDP);
	nUDP = recvfrom(fdDSUDP, bufferUDP, 5000, 0, (struct sockaddr*)&addrDSUDP, &addrlenUDP);
	if(nUDP == -1) exit(1);
	bufferUDP[nUDP] = '\0';
	//printf("Buffer UDP: %s", bufferUDP);
	processResponseUDP(bufferUDP);
}



/**
 * send message in TCP protocol
 * 
 * input:
 * 	+ message to the server
*/
void sendMessageTCP(char *message){
	char bufferTCP[128] = {0};
	char toSend[128];

	// inicializar TCP
	fdDSTCP = socket(AF_INET, SOCK_STREAM, 0); // TCP SOCKET
	if(fdDSTCP == -1) exit(1);

	memset(&hintsDSTCP, 0, sizeof hintsDSTCP);
	hintsDSTCP.ai_family = AF_INET;
	hintsDSTCP.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ipDS, portDS, &hintsDSTCP, &resDSTCP);
	if(errcode != 0) {errorMsg("Error while getting address info.\n");exit(1);}

	// sending message
	nTCP = connect(fdDSTCP, resDSTCP->ai_addr, resDSTCP->ai_addrlen);
	if(nTCP == -1) exit(1);

	nTCP = write(fdDSTCP, message, strlen(message));
	if(nTCP == -1) exit(1);
	

	puts("Antes do read");
	while(read(fdDSTCP, bufferTCP, 128) > 0){
		strcat(toSend, bufferTCP);
	}
	puts("depois do read");
	

	char* token = strtok(toSend, "\n");
	processResponseTCP(token);
	
	freeaddrinfo(resDSTCP);
	close(fdDSTCP);
}


/**000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

				AUXILIARY FUNCTIONS
				
0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000*/

/**
 * checks the input that run the program
 * 
 * input:
 * 	+ number parameters
 *  + list of parmeters
*/
void checkingInput(int argc, char *argv[]){
    for (int i = 1; i < argc - 1; i++){
		if (argv[i][0] != '-') continue;
		switch (argv[i][1]){
            case 'n':
				// to prevent cases like ./user -n -p xxxx
				if (argv[i+1][0] != '-'){ strcpy(ipDS, argv[i+1]);}
				break;   

			case 'p':
				//to prevent cases like ./user -p -n xxxx
				if (argv[i+1][0] != '-'){ strcpy(portDS, argv[i+1]);}
				break;
			default:
				errorMsg("Invalid flag.\nMust be on following format: ./user [-n DSIP] [-p DSport]");
		}
	}
}


/**
 * check if the string corresponds to a number
*/
int isNumber(char* s) {
    for (int i = 0; i < strlen(s); i++) 
        if (!isdigit(s[i])){ return 0;}
    return 1;
}


/**
 * check if the string is a alphanumeric string
*/
int isAlpha(char* str,int flag) {
	int code;
	for (int i = 0; i < strlen(str); i++) {
		code = (int)str[i];
		if (flag == 0) {
			if(!isalnum(str[i])) {
				return 0;
			}
		}
		if (flag == 1) {
			//45 is ASCII code for "-" and 95 for "_"
			if(!isalnum(str[i]) && (code != 45) && (code != 95)) {
				return 0;
			}
		}
	}
	return 1;
}


/**
 * prints the desired error message in the stderr
 * 
 * input:
 *  + message to print
 * 
*/
void errorMsg(char *errMsg){
	fprintf(stderr, "%s", errMsg);
	exit(1);
}


/**
 * check the status after one iteration
*/
void status(char uid[], char pass[], char message[], int session){
	printf("uid: %s\npass: %s\nmessage: %s\nsession: %d\n", uid, pass, message, session);
}