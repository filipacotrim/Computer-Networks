#include <stdio.h> // input & output
#include <stdlib.h> // memory alocation, control processes
#include <string.h> // string operations
#include <errno.h> //defines the integer variable errno
#include <ctype.h> // testing and mapping characters
#include "sharedMacros.h" // macros shared by the entire program
#include "commands.h" // commands 
#include "validate.h" // validate arguments
#include "tcpandudp.h" // udp and tcp connections


char buffer[BUFFER_SIZE]; // so para ler do terminal o comando
extern char message[BUFFER_SIZE]; // saber a mensagem que vamos enviar
extern char active_gid[MAX_GID]; // grupo ativo 


/**
  reads the commands given by the user
*/
int readCommands(){
  while(1){
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
        sendMessageUDP(message);
      continue;
    }

    // U N R E G I S T E R
    else if((!strcmp(token_list[0], "unregister") || !strcmp(token_list[0], "unr")) && num_tokens == 3){
      if(unregisterCommand(token_list, num_tokens))
        sendMessageUDP(message);
      continue;
    }

    // L O G I N
    else if(!strcmp(token_list[0], "login") && num_tokens == 3){ // 3 tokens -> LOG UID PASS 
      if(loginCommand(token_list, num_tokens)) {
        sendMessageUDP(message);
      }
      continue;
    }

    // L O G O U T (this command is performed locally)
    else if(!strcmp(token_list[0], "logout") && num_tokens == 1){
      if(logoutCommand(token_list, num_tokens))
        sendMessageUDP(message);
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
      sendMessageUDP(message);
      continue;
    }

    // S U B S C R I B E
    else if((!strcmp(token_list[0], "subscribe") || !strcmp(token_list[0],"s")) && num_tokens == 3){
      if(subscribeCommand(token_list, num_tokens))
        sendMessageUDP(message);
      continue;
    }

    // U N S U B S C R I B E
    else if((!strcmp(token_list[0],"unsubscribe") || !strcmp(token_list[0],"u")) && num_tokens == 2){
      if(unsubscribeCommand(token_list))
        sendMessageUDP(message);
      continue;
    }

    // M Y  G R O U P S
    else if((!strcmp(token_list[0],"my_groups") || !strcmp(token_list[0],"mgl")) && num_tokens == 1){
      if(my_groupsCommand())
        sendMessageUDP(message);
      continue;
    }

    // S E L E C T 
    else if((!strcmp(token_list[0],"select") || !strcmp(token_list[0],"sag")) && num_tokens == 2){
      selectCommand(token_list);
      continue;
    }

    // S H O W G I D
    else if((!strcmp(token_list[0],"showgid") || !strcmp(token_list[0],"sg")) && num_tokens == 1){
      //user application locally displays the GID of the selected group???
      showgidCommand();
      continue;
    }

    // U L I S T
    else if((!strcmp(token_list[0], "ulist") || !strcmp(token_list[0], "ul")) && num_tokens == 1){
      if(listCommand()){
        sendMessageTCP(message);
      } 
      continue; 
    }

    // P O S T
    else if(!strcmp(token_list[0], "post") && (num_tokens == 2 || num_tokens == 3)){ // 3 tokens -> post "text" [FName] 
      if(postCommand(token_list, num_tokens)) {
        sendMessageTCP(message);
      }
      continue;
    }


    // R E T R I E V E
    else if((!strcmp(token_list[0], "retrieve") || !strcmp(token_list[0], "r")) && num_tokens == 2){
      if(retrieveCommand(token_list, num_tokens))
        sendMessageTCP(message);
      continue;
    }

    // E X I T
    else if((!strcmp(token_list[0], "exit")) && num_tokens == 1){
      endUDPconnection(message);
      exit(EXIT_SUCCESS);
    }


  
    // D E F A U L T 
    else{
      printf("Invalid command.\n"); 
      continue;
    }   
  }
  return 0;
}
