#include <stdio.h> // input & output
#include <stdlib.h> // memory alocation, control processes
#include <string.h> // string operations
#include <unistd.h> // standard symbolic constants and types 
#include <sys/types.h> // for sockets
#include <sys/socket.h> // for sockets
#include <netinet/in.h> // for network connections
#include <arpa/inet.h> // for network connections
#include <netdb.h>  // for network connections
#include <errno.h> //defines the integer variable errno
#include <ctype.h> // testing and mapping characters
#include "sharedMacros.h" // macros shared by the entire program
#include "commands.h" // commands 
#include "validate.h" // validate arguments
#include "tcpandudp.h" // udp and tcp connections



// VARIABLES
char uid[UID_SIZE] = ""; // user's uid
char pass[PASS_SIZE] = ""; // user's password
char message[BUFFER_SIZE] = ""; // message to send to server
int session = LOGGED_OUT; // logged in or logged out
int activeGid = DEACTIVATED; // active group ID
char gid[MAX_GID] = ""; // group's ID
char active_gid[MAX_GID] = ""; // active group ID
char activeGname[GNAME_SIZE] = ""; // group's name
char text[Tsize] = ""; // text to send to server
char Fname[] = ""; // file name to sent do server
int mid = 1;


// FUNCTIONS
int registerCommand(char *token_list[], int num_tokens); // command register
int unregisterCommand(char *token_list[], int num_tokens); // command unregister
int loginCommand(char *token_list[], int num_tokens); // command login
int logoutCommand(char *token_list[], int num_tokens);
int subscribeCommand(char *token_list[], int num_tokens); // command subscribe
int unsubscribeCommand(char *token_list[]); // unsubscribe command
int showuidCommand(char *token_list[], int num_tokens); // showuid command
int showgidCommand(); //showgid command
int my_groupsCommand(); //my_gorups command
int selectCommand(char *token_list[]); // select command
int listCommand(); // list command
int postCommand(char *token_list[],int num_tokens); // select command
int retrieveCommand(char *token_list[], int num_tokens); // retrieve command




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
	sprintf(message, "REG %s %s\n", token_list[1], token_list[2]);
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

	if(strcmp(token_list[1],uid) == 0) {
		//logout
		logoutCommand(0,1);

		sprintf(message, "UNR %s %s\n", token_list[1], token_list[2]);
		return 1;
	}
	sprintf(message, "UNR %s %s\n", token_list[1], token_list[2]);
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
	//delete current uid 
	strcpy(uid,"");
	strcpy(active_gid,"");
	activeGid = DEACTIVATED;
	session = LOGGED_OUT;
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
 * show's the id of the activated group
 * 
 * input:
 *	 + list of parameters read in the input 
 *	 + number of parameters read in the input
 */
int showgidCommand(){
	if(session == LOGGED_OUT){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	if(activeGid == DEACTIVATED) {
		printf("You must have a selected group ro perform this action first.\n");
		return 0;
	}
	printf("GID: %s\n", active_gid);
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
	strcpy(activeGname, token_list[2]);
	sprintf(message, "GSR %s %s %s\n", uid, gid, activeGname);
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
 * post command
 *
 * input:
 * 	 + token_list
 *   + num_tokens
 */
int postCommand(char *token_list[],int num_tokens) {
	char fname[24];
	if(session == LOGGED_OUT ){
		printf("You must login to perform this action first.\n");
		return 0;
	}
	if(activeGid == DEACTIVATED){
		printf("You must have a group selected to perform this action first.\n");
		return 0;
	}

	if (num_tokens == 2) {
		int socket = initializeTCP();
		sprintf(message, "PST %s %s %ld %s\n", uid, active_gid,strlen(token_list[1]),token_list[1]);
		writeSocketTCP(message,socket);
		sendMessageTCP(socket);
	}

	else if (num_tokens == 3) {
		strcpy(fname,token_list[2]);
		if(!isValidFileName(token_list[2])) {
			printf("Invalid text or file name.\n");
			return 0;
		}
		
		int socket = initializeTCP();

		FILE *fptr;
    
		// Open file
		fptr = fopen(fname, "rb");
		if (fptr == NULL)
		{
			printf("Failed to open file.\n");
			return 0;
		}

		size_t fsize = getFileSize(fname);
		int SIZE = fsize;
		char *data = (char*)calloc(1024,sizeof(char));
		char *messageToPost = (char*)calloc(1024,sizeof(char));
		int dim = 0;
		int soma = 0;
		sprintf(messageToPost, "PST %s %s %ld %s %s %ld ", uid, active_gid, strlen(token_list[1]), token_list[1], 
		fname, fsize);
		int str_size = strlen(messageToPost);
		send(socket, messageToPost, str_size, 0);
		
		free(messageToPost);

		while(fsize > 1024) {
            dim = fread(data,sizeof(char),1024,fptr);
            send(socket, data, dim, 0);
            memset(data,0,dim);
            fsize -= dim;
            soma += dim;
        }
        dim = fread(data,sizeof(char),fsize,fptr);
		soma += dim;
		send(socket, data, dim, 0);
		send(socket, "\n", 1, 0);
		printf("Fsize: %d Dim: %d\n", SIZE, soma);
	
		memset(data,0,fsize);
		sendMessageTCP(socket);
		fclose(fptr);
		free(data);

	}	
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
	int socket = initializeTCP();
	sprintf(message, "ULS %s\n", active_gid);
	writeSocketTCP(message,socket);
	sendMessageTCP(socket);
	return 1;
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
	int socket = initializeTCP();
	char * messageToPost = (char*)calloc(240,sizeof(char));
	sprintf(messageToPost, "RTV %s %s %s\n", uid, active_gid, token_list[1]);
	writeSocketTCP(messageToPost,socket);
	sendMessageTCP(socket);
	free(messageToPost);
	return 1;
}

