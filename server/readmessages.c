#include <stdio.h> // input & output
#include <stdlib.h> // memory alocation, control processes
#include <string.h> // string operations
#include <errno.h> //defines the integer variable errno
#include <ctype.h> // testing and mapping characters
#include "commands.h"
#include "profunctions.h"
#include "validate.h"
#include "sharedMacros.h"

#define MAX_TOKENS_COMMAND 10 // isto vai sair daqui

/***
 * reads the message and sends it to the correct command
 *
 * input:
 * 	+ message (msg)
*/
char* readCommands(char *msg){
	char* answer; // o free faz se no sockets.c

	int num_tokens = 0;
	char* token_list[MAX_TOKENS_COMMAND];
	char* token = strtok(msg, " \n");
	int result;

	// KNOW THE TOKENS WRITTEN
	while (token != NULL && num_tokens < MAX_TOKENS_COMMAND){
		token_list[num_tokens++] = token;
		token = strtok(NULL, " \n");
	}


	// R E G I S T E R
	if(!strcmp(token_list[0], "REG") && num_tokens == 3){ // 3 tokens -> reg uid pass
		answer = malloc(sizeof(char)*100);
		result = registerCommand(token_list, num_tokens);
		if( result == 1 ){
			sprintf(answer, "RRG OK\n");
		}
		else if( result == 2 ){
			sprintf(answer,"RRG DUP\n");
		}
		else{
			sprintf(answer, "RRG NOK\n");
		}
		return answer;
	}

	// U N R E G I S T E R
	else if(!strcmp(token_list[0], "UNR") && num_tokens == 3){ //UNR UID pass
		answer = malloc(sizeof(char)*100);
		result = unregisterCommand(token_list, num_tokens);
		if( result == 1){
			sprintf(answer, "RUN OK\n");
		}else{
			sprintf(answer, "RUN NOK\n");
		}
		return answer;
	}

	// L O G I N
	else if(!strcmp(token_list[0], "LOG") && num_tokens == 3){ // 3 tokens -> LOG UID PASS
		answer = malloc(sizeof(char)*100);
		result = loginCommand(token_list, num_tokens);
		if(result == 1){
			sprintf(answer, "RLO OK\n");
		}
		else{ // erro
			sprintf(answer, "RLO NOK\n");
		}
		return answer;
	}

	// L O G O U T
	else if(!strcmp(token_list[0], "OUT") && num_tokens == 3){
		answer = malloc(sizeof(char)*100);
		result = logoutCommand(token_list, num_tokens);
		if(result == 1){
			sprintf(answer, "ROU OK\n");
		}
		else{ // erro
			sprintf(answer, "ROU NOK\n");
		}
		return answer;
	}

	// G R O U P S (nao aparece bem no user)
	else if(!strcmp(token_list[0],"GLS") && num_tokens == 1){
		answer = GroupsCommand(token_list);
		//printf("%s", answer);
		return answer;
	}

	// S U B S C R I B E
	else if((!strcmp(token_list[0], "GSR")) && num_tokens == 4){
		answer = malloc(sizeof(char)*100);
		result = subscribeCommand(token_list, num_tokens);
		if(result == 1){
			sprintf(answer, "RGS OK\n");
		}
		else if(result == 2){ // uid invalid
			sprintf(answer, "E_USR\n");
		}
		else if(result == 3){ // gid invalid
			sprintf(answer, "E_GNAME\n");
		}
		else if(result == 4){ // if the group list is full
			sprintf(answer, "E_FULL\n");
		}
		else if(result == 5){ // wrong name
			sprintf(answer, "E_GNAME\n");
		}
		else{ // erro
			sprintf(answer, "RGS NOK\n");
		}
		return answer;
	}

	// U N S U B S C R I B E
	else if(!strcmp(token_list[0],"GUR") && num_tokens == 3){
		answer = malloc(sizeof(char)*100);
		result = unsubscribeCommand(token_list, num_tokens);
		if( result == 1 ){
			sprintf(answer, "RGU OK\n");
		// mandar a mensagem po cliente que tudo correu bem
		}
		else{ // erro
			sprintf(answer, "RGU NOK\n");
		// mandar a mensagem po cliente que nao correu bem
		}
		return answer;
	}

	// M Y  G R O U P S (nao chega bem ao user)
	else if(!strcmp(token_list[0],"GLM") && num_tokens == 2){
		/*answer = my_groupsCommand(token_list, num_tokens);
		return answer;*/
		return my_groupsCommand(token_list, num_tokens);
	}

	// E X I T
	else if((!strcmp(token_list[0], "exit")) && num_tokens == 1){
		// nem sei o que acontece
	}

	// D E F A U L T
	else{
		printf("Invalid command.\n");
	}

	return 0;
}
