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
#include "readCommands.h" // read commends from the user
#include "validate.h" // validate arguments


int fdDSUDP, fdDSTCP, errcode;

// UDP 
ssize_t nUDP;
socklen_t addrlenUDP;
struct addrinfo hintsDSUDP, *resDSUDP;
struct sockaddr_in addrDSUDP;

// TCP
ssize_t nTCP;
socklen_t addrlenTCP;
struct addrinfo hintsDSTCP, *resDSTCP;
struct sockaddr_in addrDSTCP;

// port and ip adress 
char portDS[PORT_SIZE]; // por number
char ipDS[IP_SIZE]; // ip address to connect

// external global variables
extern int session; // logged in or not
extern char gname[GNAME_SIZE]; // nome do grupo 



void portandIP(char *port, char *ip); // defines the ip adress and port 
void initializeUDP(); // initializes the udp connection with the server
void sendMessageUDP(char *msg); // sending message in udp protocol
void sendMessageTCP(char *msg); // sending message in tcp protocol
void processResponseTCP(char *msg); // process the response given in tcp protocol
void processResponseUDP(char *msg); // process the response given in udp protocol
void endUDPconnection(); // ends udp connection with the server



/**
  * defines the ip adress and port
  * input:
  *  + port
  *  + ip
  */
void portandIP(char* port, char *ip){
  strcpy(portDS, port); // they don't change across the program
  strcpy(ipDS, ip); // doesn't change across the program
}



/**
 * initializes a udp connection with the server
 */
void initializeUDP(){
  // inicializar UDP
  fdDSUDP = socket(AF_INET, SOCK_DGRAM, 0); // UDP SOCKET
  if(fdDSUDP == -1) exit(1);

  memset(&hintsDSUDP, 0, sizeof hintsDSUDP);
  hintsDSUDP.ai_family = AF_INET;
  hintsDSUDP.ai_socktype = SOCK_DGRAM;

  errcode = getaddrinfo(ipDS, portDS, &hintsDSUDP, &resDSUDP);
  if(errcode != 0) {printf("Error while getting address info.\n");exit(1);}

}



/**
 * send message in UDP protocol
 * 
 * input:
 *  + message to the server
*/
void sendMessageUDP(char *msg){
  char bufferUDP[5000];
  // sending the message
  nUDP = sendto(fdDSUDP, msg, strlen(msg), 0, resDSUDP->ai_addr, resDSUDP->ai_addrlen);
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
 *  + message to the server
*/
void sendMessageTCP(char *msg){
  char bufferTCP[128] = {0};
  char *toSend = (char*) calloc(128,sizeof((char*)128 + 1));

  // inicializar TCP
  fdDSTCP = socket(AF_INET, SOCK_STREAM, 0); // TCP SOCKET
  if(fdDSTCP == -1) exit(1);

  memset(&hintsDSTCP, 0, sizeof hintsDSTCP);
  hintsDSTCP.ai_family = AF_INET;
  hintsDSTCP.ai_socktype = SOCK_STREAM;

  errcode = getaddrinfo(ipDS, portDS, &hintsDSTCP, &resDSTCP);
  if(errcode != 0) {printf("Error while getting address info.\n");exit(1);}

  // sending message
  nTCP = connect(fdDSTCP, resDSTCP->ai_addr, resDSTCP->ai_addrlen);
  if(nTCP == -1) exit(1);

  nTCP = write(fdDSTCP, msg, strlen(msg));
  if(nTCP == -1) exit(1);
  //printf("msg : %s\n",msg);

  while(read(fdDSTCP, bufferTCP, 128) > 0){
      strcat(toSend,bufferTCP);
  }


  char* token = strtok(toSend, "\n");
  //printf("toSend: %s\n",toSend);
  processResponseTCP(token);
  freeaddrinfo(resDSTCP);
  free(toSend);
  close(fdDSTCP);
}




/**
 * process the message given by the server in TCP protocol
 * 
 * input:
 *  + message from the server
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
  } else if(!strcmp(token_list1[0],"RPT")) { //POST
    if(!strcmp(token_list1[1], "OK")){
      printf("Succesfully Posted.\n");
  } else if(!strcmp(token_list1[1], "NOK"))
      printf("Invalid text or file name.\n");
  }
}




/**
 * process the message given by the server in UDP messages 
 * 
 * input:
 *  + message from the server
*/
void processResponseUDP(char *msg){
  int num_tokens = 0;
  int counter = 0;
  char* token_list[MAX_TOKENS_RES]; 
  char* token = strtok(msg, " \n");
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
 * ends the UDP connection with the server
 */
void endUDPconnection(){
	freeaddrinfo(resDSUDP); // POSSIVELMENTE FAZER NO FICHEIRO DAS SOCKETS
  close(fdDSUDP); // POSSIVELMENTE FAZZER NO FICHEIRO DAS SOCKETS
}