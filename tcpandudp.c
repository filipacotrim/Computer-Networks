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

#define DIDNT_READ_SLASH 1
#define ALL_GOOD 0

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

int min(int a, int b){
  return (a > b) ? b : a;
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
  if(nUDP == -1){
    printf("Failed to send message.\n");
    return;
  } 
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

  //ler o opcode + status
  //char header[7] = {0};
  char op[4] = {0};
  char status[4] = {0};
  char mid[4] = {0};
  char uid[5] = {0};
  char tsize[3] = {0};
  char fname[24] = {0};
  char fsize[10] = {0};
  char text[240] = {0};
  char slash[2] = {0};
  char N[2] = {0} ;
  char c[1] = {0};
  char save[1] = {0};
  char toConcat[4] = {0};


  int g = read(fdDSTCP,op,4);
  op[g] = '\0';
  //printf("op:%s.\n",op);

  if(!strcmp(op,"RUL ")) { //ULIST
    memset(op,0,4);
    read(fdDSTCP,status,3);
    printf("status: %s.\n",status);
    if(!strcmp(status,"OK ")) {
      char gname[24] = {0};
      int count=0;
      memset(status,0,3);
      read(fdDSTCP,c,1);
      while(count < 24) { 
        read(fdDSTCP,c,1);
        if(strcmp(c," ")) {
          //printf("c: %s\n",c);
          strcat(gname,c);
          count++;
          //printf("here1\n");
        }
        else {
          printf("User(s) is Group %s :\n",gname);
          break;
        }
      }
      memset(gname,0,24);
      char uid[5] = {0};
      int n;
      int users = 0;
      while((n = read(fdDSTCP,uid,6)) > 0) {
        printf("UID: %s\n",uid);
        users = 1;
      }
      if (users == 0) {
        printf("There are no users in this group.\n");
      }

    }
    if (!strcmp(status,"NOK")) {
      printf("This group does not exist.\n");
    }
  }

  if (!strcmp(op,"RPT ")){ //POST
    printf("entra aqui\n");
    memset(op,0,4);
    read(fdDSTCP,status,3);
    printf("status: %s.\n",status);
    if(!strcmp(status,"NOK")) {
      printf("Post unsuccessful.\n");
    }
    else {
      printf("Successfully posted!\n");
    }
    memset(status,0,3);
  }

  if (!strcmp(op,"RRT ")) { //RETRIEVE
    //printf("OPCode: %s.\n",op);
    //sscanf(header,"%s %s",op,status);
    
    //printf("Status: %s.\n",status);

    memset(op,0,4);
    read(fdDSTCP,status,3);

    //printf("Op: %s. Status: %s. \n",op,status);
    if(!strcmp(status,"OK ")) {
      memset(status,0,3);
      int flag = ALL_GOOD;
      read(fdDSTCP,N,2);
      int numMsg = atoi(N);
      //espaco depois do N
      read(fdDSTCP, c, 1);
      for (int i = 1; i <= numMsg; i++) {
        //printf("FLAG %d\n",flag);
        if(flag==ALL_GOOD){
          //printf("Entrei no All Good.\n");
          read(fdDSTCP,mid,5);
          printf("MID: %s",mid);
          memset(mid,0,strlen(mid));
        
        } else if(flag == DIDNT_READ_SLASH){
          //printf("Entrei no Didnt Read.\n");
          read(fdDSTCP, toConcat, 4);
          strcat(mid, save); //este strcat estraga o toConcat
          strncat(mid, toConcat, 4);
          printf("MID: %s",mid);
          memset(mid,0,strlen(mid));
          memset(save, 0, 1);
          //read(fdDSTCP, c, 1);
        }
        int m = read(fdDSTCP,uid,6);
        uid[m] = '\0';
        printf("/UID: %s",uid);
        memset(uid,0, strlen(uid));
	      memset(c, 0, 1);
        int count = 0;
        while(count <= 3) { 
          read(fdDSTCP,c,1);
          //printf("c: %s.\n",c);
          if(!strncmp(c," ",1)) {
            
            printf("/tsize: %s ",tsize);
            break;
          }
          else {
            strncat(tsize,c,1);
            count++;
	          memset(c, 0, 1);
          }
        }
        int size = atoi(tsize);
        memset(tsize,0,3);
        int k;
        k = read(fdDSTCP,text,size);
        text[k] = '\0';
        printf("/text: %s ",text);
        //memset(text,0,size);
        read(fdDSTCP,c,1);  //espaco depois do text
	      //printf("C: %s.\n", c);    
        read(fdDSTCP, slash, 1); //ler mais um para ver se tem o /
        /**
         * 3 possibilidades:
         *    - lemos um / -> continuamos normalmente
         *    - lemos um \n -> e porque ja acabamos a mensagem e nao ha mais nada a seguir, damos continue
         *    - nao lemos nenhum dos dois -> e porque lemos o primeiro digito do proximo MID
         *                                   temos de o guardar e concatenar com os proximos 3 digitos do MID
         * 
         */
        //printf("Slash: %s.\n", slash);
        if(!strcmp(slash, "/")){
          flag = ALL_GOOD;
	        memset(c,0,1);
          read(fdDSTCP,c,1);
	        //printf("C: %s.\n",c);
          //continuar a ler as cenas do ficheiro
          FILE *newFile;
          int count2 = 0;
          while(count2 <= 24) { 
            read(fdDSTCP,c,1);
            if(strncmp(c," ",1)) {
              //printf("c: %s\n",c);
              strncat(fname,c,1);
              count2++;
              //printf("here1\n");
            }
            else {
              printf("/Fname %s :",fname);
              break;
            }
          }
	        //printf("FNAME: %s.", fname);
          int count3 = 0;
          while(count3 < 10) { 
            read(fdDSTCP,c,1);
            //printf("C do Fsize: %s.\n",c);
            if(!strncmp(c," ",1)) {
              
              printf("/fsize: %s\n",fsize);
              break;
            }
            else {
              strncat(fsize,c,1);
              count3++;
            }
          }

          newFile = fopen(fname, "wb"); // criar o novo ficheiro
          
          if(newFile == NULL){
            perror("error");
            printf("\n"); 
            return;
          } // em caso de erro

          char data[1024] = {0};
          int Fsize = atoi(fsize);
          int dim; 
          int soma = 0;
         
          //do{
            //int num = min(Fsize, sizeof(data));
            //int offset = 0;
            //dim = read(fdDSTCP, data, 1024);
            //do{
              //size_t written = fwrite(&data[offset], 1, num-offset, newFile);
              //offset += written;
            //} while(offset < num);
            //memset(data, 0, dim);
	          //soma += dim;
	          //Fsize -= num;
            //}while(Fsize > 0);
	        //memset(data, 0, dim);

          while(Fsize > 1024) {
            dim = read(fdDSTCP,data,1024);
            fwrite(data, 1, 1024, newFile); // escrever a data para o novo ficheiro
            memset(data,0,dim);
            Fsize -= dim;
            soma += dim;
            //printf("Fsize: %d\n",Fsize);
          }
          //printf("Fsize: %s Dim: %d\n",fsize,soma);
          dim = read(fdDSTCP,data,Fsize);
	        soma += dim;
	        printf("Fsize: %s Dim: %d\n", fsize, soma);
          //sum += strlen(data);
          //printf("sum: %d\n",soma);
          //soma += fwrite(data, 1, Fsize, newFile); // escrever a data para o novo ficheiro
          memset(data,0,Fsize);
          //fwrite("\n",1,1,newFile);
          fclose(newFile);
          memset(fname,0,24);
          memset(fsize,0,10);
          read(fdDSTCP,c,1);
          memset(c,0,1);
          memset(slash,0,1);

        } else if(!strcmp(slash, "\n")){
            continue; //ver se lemos um \n
        } else{
          printf("\n");
          strcpy(save, slash);
          flag = DIDNT_READ_SLASH;
        }
      }
    }
    else if(!strcmp(op,"NOK")) {
      printf("nok");
    }
    else if(!strcmp(op,"EOF")) {
      printf("eof");
    }

  }
  freeaddrinfo(resDSTCP);

  close(fdDSTCP);
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
    if(!strcmp(status, "OK"))
      //ok  
      printf("Successfully subscribed to group %s.\n", gname);
    else if(!strcmp(status, "NEW"))
      //create new group
      printf("Successfully created and subscribed group %s.\n", gname);
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
}




/**
 * ends the UDP connection with the server
 */
void endUDPconnection(){
	freeaddrinfo(resDSUDP); // POSSIVELMENTE FAZER NO FICHEIRO DAS SOCKETS
  close(fdDSUDP); // POSSIVELMENTE FAZZER NO FICHEIRO DAS SOCKETS
}
