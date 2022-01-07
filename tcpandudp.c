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
extern char activeGname[GNAME_SIZE]; // nome do grupo 



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
  //printf("msg recebida: %s",msg);
  char *toSend = (char *) calloc(strlen(msg)+1,sizeof(char));
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
  int n;
  while((n = read(fdDSTCP, bufferTCP, 128) > 0)){
    strncat(toSend,bufferTCP, 128);
    //printf("Buffando :%s\n",bufferTCP);
    //memset para os casos onde, por exemplo, buffer ficava com o "rim" do meu nome
    memset(bufferTCP,0,128);
  }

  //mudei, pois ultima mensagem de todas nÃ£o aparecia
  
  //printf("toSend: %s\n",toSend);
  toSend[strlen(toSend)] = '\0';

  processResponseTCP(toSend);
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
  int num_msgs, i=0;
  char opcode[3] = {0};
  char status[3] = {0};
  char numMSG[3] = {0};
  char mid[4] = "novo";
  char uid[5] = {0};
  char gname[24] = {0};
  char tsize[3] = {0};
  char text[240] = {0};
  char token_temp[240] = {0};
  char slash[4] = {0};

  sscanf(msg,"%s %[^\n]", opcode, msg);
  if(!strcmp(opcode, "RRT")){ //RETRIEVE
      sscanf(msg,"%s %[^\n]", status, msg);
      if(!strcmp(status, "OK")){
          sscanf(msg,"%s %[^\n]", numMSG, msg);
          num_msgs = atoi(numMSG);
          printf("You currently have %d unread messages :\n",num_msgs);
          for(int j=1; j<=num_msgs;j++){
              int textSize;
              if(!strcmp(mid,"novo")){
                  memset(mid, 0, 4);
                  sscanf(msg,"%s %[^\n]", mid, msg);
              }
                  
              printf("MID: %s", mid);
              sscanf(msg,"%s %[^\n]", uid, msg);
              printf(" UID: %s", uid);
              sscanf(msg,"%s %[^\n]", tsize, msg);
              printf(" Tsize: %s", tsize);
              textSize = atoi(tsize);
              
              while(i<textSize){
                  sscanf(msg,"%s %[^\n]", token_temp, msg);
                  strncat(text, token_temp, strlen(token_temp));
                  i += strlen(token_temp);
                  memset(token_temp, 0, 240);
                  
                  if(i<textSize){
                      strcat(text, " ");
                      i++;
                  }
              } 
              printf(" Text: %s", text);
              
              if(!strcmp(slash, text)){
                break;
              }

              sscanf(msg, "%s %[^\n]", slash, msg);
              
              if(!strcmp(slash, "/")){
                  char fname[24] = {0};
                  char fsize[10] = {0};
                  //token vai ter de ter o mm tamanho que o text
                  char token_file[240] = {0};
                  int fileSize, k=0;
                  //abrir ficheiro para descobrir quanto temos de alocar
                  sscanf(msg, "%s %[^\n]", fname, msg);
                  printf(" Fname: %s", fname);
                  sscanf(msg, "%s %[^\n]", fsize, msg);
                  fileSize = atoi(fsize);
                  
                  FILE *newPic = fopen(fname, "wb"); // criar o novo ficheiro
                  if(newPic == NULL){
                      perror("Cannot open file."); 
                      exit(1);
                  } // em caso de erro
                  
                  printf(" Fsize: %d\n", fileSize);
                  while(k<fileSize){
                      sscanf(msg,"%s %[^\n]", token_file, msg);
                      //printf("%s\n", token_file);
                      k += strlen(token_file);
                      fwrite(token_file, 1, strlen(token_file), newPic); // escrever a data para o novo ficheiro
                      memset(token_file, 0, 240);
                      if(k<fileSize){
                          fwrite(" ", 1, 1, newPic);
                          k++;
                      }
                  }
                  fclose(newPic);
                  k = 0;
              } else{
                  strcpy(mid, slash);
              }
          i = 0;
          printf("\n");
          memset(text, 0, 240);
          } 
      } else if(!strcmp(status,"NOK")){
          printf("There was a problem with the retrieve request.\n");
      } else if(!strcmp(status, "EOF")){
          printf("There are no messages available.\n");
      }
  }
    
  else if(!strcmp(opcode, "RUL")) { //ULIST
      sscanf(msg,"%s %[^\n]", status, msg);
      if(!strcmp(status, "OK")) {
        int n = sscanf(msg,"%s %[^\n]", gname, msg);
        if (n == 1) {
          printf("This group has no subscribed users.\n");
        }
        else {
          printf("Users in group %s:\n",gname);
          n = sscanf(msg,"%s %[^\n]", uid, msg);
          printf("UID: %s\n",uid);
          while (n > 1) {
            n = sscanf(msg,"%s %[^\n]", uid, msg);
            printf("UID: %s\n",uid);
          }
           
        }
      } else if(!strcmp(status, "NOK")) 
          printf("This group does not exist.\n");
  } 
  else if(!strcmp(opcode,"RPT")) { //POST
      sscanf(msg,"%s %[^\n]", status, msg);
      if(!strcmp(status, "NOK"))
        printf("Invalid text or file name.\n");
      else {
        printf("Successfully posted!\n");
      }
  }
  
}



/**
 * process the message given by the server in UDP messages 
 * 
 * input:
 *  + message from the server
*/
void processResponseUDP(char *msg){
  char opcode[3] = {0};
  char status[3] = {0};
  char mid[4] = {0};
  char gid[2] = {0};
  char gname[24] = {0};
  char N[2] = {0};
  sscanf(msg,"%s %[^\n]", opcode, msg);
  
  if(!strcmp(opcode, "RRG")){ // REGISTER
    sscanf(msg,"%s %[^\n]", status, msg);
    //registration successfull
    if(!strcmp(status, "OK"))
      printf("Successfully registered user.\n");
    //user already exists
    else if(!strcmp(status, "DUP"))
      printf("This ID is already belongs to another user.\n");
    //too many users registered
    else if(!strcmp(status, "NOK"))
      printf("Registration failed. Too many users registered already.\n");
    
  } else if(!strcmp(opcode, "RUN")){ // UNREGISTER 
    sscanf(msg,"%s %[^\n]", status, msg);
    if(!strcmp(status, "OK"))
      //unregistration successfull
      printf("Successfully unregistered user.\n");
    else if(!strcmp(status, "NOK"))
      //invalid UID or incorrect pass
      printf("Invalid UID or incorrect password. Failed to unregistered.\n");
  
  } else if(!strcmp(opcode, "RLO")){ // LOGIN
    sscanf(msg,"%s %[^\n]", status, msg);
    if(!strcmp(status, "OK")){
      //login successfull
      printf("Successfully logged in.\n");
      session = LOGGED_IN;
    } else if(!strcmp(status, "NOK"))
      //invalid UID or incorrect pass
      printf("Invalid UID or incorrect password. Failed to log in.\n");

  } else if(!strcmp(opcode, "ROU")){ // LOGOUT
    sscanf(msg,"%s %[^\n]", status, msg);
    if(!strcmp(status, "OK")){
      //logout successfull
      printf("Successfully logged out.\n");
      session = LOGGED_OUT;
    } else if(!strcmp(status, "NOK"))
      //invalid UID or incorrect pass
      printf("Invalid UID or incorrect password. Failed to log out.\n");

  } else if(!strcmp(opcode, "RGL")){ // GROUPS
    sscanf(msg,"%s %[^\n]", N, msg);
    printf("There are currently %s groups.\n", N);
    int totalGroups = atoi(N);
    //printf("N: %d\n",totalGroups);
    for (int i = 0; i < totalGroups;i++) {
      sscanf(msg,"%s %[^\n]", gid, msg);
      printf("GID: %s ",gid);
      sscanf(msg,"%s %[^\n]", gname, msg);
      printf("/Gname: %s ",gname);
      sscanf(msg,"%s %[^\n]", mid, msg);
      printf("/MID: %s\n",mid);
    }
  } else if(!strcmp(opcode, "RGS")){ // SUBSCRIBE
    sscanf(msg,"%s %[^\n]", status, msg);
    if(!strcmp(status, "OK")){
      //ok
      printf("Successfully subscribed to group %s.\n", activeGname);
    }
    else if(!strcmp(status, "NEW")){
      //create new group
      printf("Successfully created and subscribed group %s.\n", activeGname);
    }
    else if(!strcmp(status, "E_USR"))
      //invalid UID
      printf("Invalid user ID.\n");
    else if(!strcmp(status, "E_GRP"))
      //invalid GID
      printf("Invalid group ID.\n");
    else if(!strcmp(status, "E_GNAME"))
      //invalid GNAME
      printf("Invalid group name.\n");
    else if(!strcmp(status, "E_FULL"))
      //max reached
      printf("Error. Maximum number of groups has already been reached.\n");
    else if(!strcmp(status, "NOK"))
      //other error
      printf("Failed to subscribe/create group.\n");


  } else if(!strcmp(opcode, "RGU")){ // UNSUBSCRIBE
    sscanf(msg,"%s %[^\n]", status, msg);
    if(!strcmp(status, "OK"))
      //ok  
      printf("Successfully unsubscribed from group.\n");
    else if(!strcmp(status, "E_USR"))
      //invalid UID
      printf("Invalid user ID.\n");
    else if(!strcmp(status, "E_GRP"))
      //invalid GID
      printf("Invalid group ID.\n");
    else if(!strcmp(status, "NOK"))
      //other error
      printf("Error. Failed to unsubscribe from group.\n");

  } else if(!strcmp(opcode, "RGM")){ // MY_GROUPS
    sscanf(msg,"%s %[^\n]", N, msg);
    printf("You are currently subscribed to %s groups.\n", N);
    int totalMgl = atoi(N);
    for (int i = 0; i < totalMgl;i++) {
      sscanf(msg,"%s %[^\n]", gid, msg);
      printf("GID: %s ",gid);
      sscanf(msg,"%s %[^\n]", gname, msg);
      printf("/GName: %s ",gname);
      sscanf(msg,"%s %[^\n]", mid, msg);
      printf("/MID: %s\n",mid);
    }
  }
  memset(opcode, 0, 3);
  memset(status, 0, 3);
  memset(gname, 0, 24);
  memset(mid, 0, 4);
  memset(gid, 0, 2);
  memset(N, 0, 2);  
}




/**
 * ends the UDP connection with the server
 */
void endUDPconnection(){
	freeaddrinfo(resDSUDP); // POSSIVELMENTE FAZER NO FICHEIRO DAS SOCKETS
  close(fdDSUDP); // POSSIVELMENTE FAZZER NO FICHEIRO DAS SOCKETS
}