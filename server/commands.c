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
#include <dirent.h> // for directory functions
#include "profunctions.h"
#include "validate.h"
#include "readmessages.h"
#include "sharedMacros.h"
#include "filefunctions.h"



/**
 * evaluates the uid and pass
 * creates a user file, if there already is one (dup) it stops
 *
 * output:
 *  + 1 (ok)
 *  + 2 (dup)
 *  + 0 (error)
*/
int registerCommand(char *token_list[], int num_tokens)
{
	int result1, result2;
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		return 0; // error
	}
	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		return 0;
	}

    result1 = CreateUserDir(token_list[1]);
    if( result1 == 2 ){return 2;} // DUP
    result2 = CreateUserPass(token_list[1], token_list[2]);
    if( result1 == 0 || result2 == 0 ){ printf("erro a criar a pass");return 0;} // erro
    return 1; // all good
}



/**
 * vai verificar se o user existe
 * se o user existir vai verificar se a pass e a correta
 * se ambos estiverem bem apaga a diretoria e os ficheiros
 *
 *
*/
int unregisterCommand(char *token_list[], int num_tokens) // UNR UID pass
{
	int num_groups;
	if(CheckIfUserExists(token_list[1]) == 0 ) return 0; // check if user exists
	if(CheckUserPass(token_list[1], token_list[2]) == 0) return 0;
	GROUPLIST *list = malloc(sizeof(GROUPLIST));
	num_groups = ListGroupsDir(list); // knowing existing groups

	// for every existing group
	for(int i=0; i<num_groups; i++){
		// if user in it unsubscribe
		if(CheckIfInGroup(list->group_no[i], token_list[1]) == 1){
			//printf("esta no grupo: %s", list->group_no[i]);
			if(RemoveUserFromGroup(list->group_no[i], token_list[1]) == 0){
				printf("ERROR: while removing user from group\n");
			}
		}
	}

	free(list);
	// deleting the directory of the user
	if(DelUserDir(token_list[1]) == 0) return 0;
	return 1; // all good
}


/**
 * login command
 * checks if the password is correct and log's in the uid
 *
 * output:
 * 	+ 1 (login)
 *  + 0 (error)
*/
int loginCommand(char *token_list[], int num_tokens)
{
	int result;
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		return 0;
	}

	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		return 0;
	}

	result = CheckUserPass(token_list[1], token_list[2]); // checks uid and pass
	if( result == 0) {return 0;} // erro
	return CreateLoginUser(token_list[1]);
}



/**
 * logout command
 * checks if the password is correct, if the user is logged in
 * logs out
 *
 * output:
 * 	+ 1 (logout)
 *  + 0 (error)
*/
int logoutCommand(char *token_list[], int num_tokens)
{
	//check UID
	if(strlen(token_list[1]) != UID_SIZE || !isNumber(token_list[1])){ //UID must have 5 characters
		return 0;
	}

	//check password
	if(strlen(token_list[2]) != PASS_SIZE || !isAlpha(token_list[2],0)){ //password must have 8 characters
		return 0;
	}

	if( CheckLoggedIn(token_list[1]) == 0 ) return 0; // not logged in
	if( CheckUserPass(token_list[1], token_list[2]) == 0 ) return 0; // not the correct password
	if( DeleteLoginUser(token_list[1]) == 0 ) return 0; // error deleting

	return 1; // all good
}



/**
 * subscribe command
 * checks if the UID exists, if gid == 00 creates new group with gname
 * otherwise joins the UID to the gid reffered
 *
 * output:
 * 	+ 1 (subscribed)
 *  + 2 (uid doesn't exist)
 *  + 3 (gid doens't exist)
 *  + 0 (error)
*/
int subscribeCommand(char *token_list[], int num_tokens) // GSR uid gid gname
{
	char *zero = "00";

	if(CheckIfUserExists(token_list[1]) == 0) {return 2;} // the user doesn't exist
	if(CheckLoggedIn(token_list[1]) == 0) {return 0;}

	// if GID == 00
	if(!strcmp(token_list[2], zero))
	{
		int res, num_groups=0;
		char* GID;
		num_groups = KnowHowManyGroups();
		//printf("%d\n", num_groups);
		GID = KnowTheNewGid(num_groups);
		if(GID == NULL){return 4;} // not possible to create another group
		res = CreateGroup(GID, token_list[3], token_list[1]);
		if(res == 1) {num_groups++;} // if the groups was created
		free(GID);
		return res;
	}
	else // if GID != 00
	{
		int res = CheckIfGroupExists(token_list[2], token_list[3]);
		if(res == 0){return 0;} // group doens't exist
		else if(res == 2){return 5;} // wrong group name
		return JoinGroup(token_list[2], token_list[1]);
	}
	return 1; // all good
}



/**
 * unsubscribe command
 * checks if the groups exists, and if the user is in the group
 * removes the user from the group
 *
 * ouput:
 *  + 1 (unsubsribed)
 *  + 0 (error)
*/
int unsubscribeCommand(char *token_list[], int num_tokens) // GUR UID GID
{
	if(strlen(token_list[2]) > MAX_GID || !isNumber(token_list[2])){ //GID must be a 2-digit number
		return 0;
	}
	if(CheckIfInGroup(token_list[2], token_list[1]) == 0) return 0; // user not in group
	if(RemoveUserFromGroup(token_list[2], token_list[1]) == 0) return 0; // error removing
	return 1; // all good
}



/**
 * my groups commmand
 *
 * output:
 * 	answer
*/
char*my_groupsCommand(char *token_list[], int num_tokens){
	GROUPLIST *list = malloc(sizeof(GROUPLIST));
	int num_groups = ListGroupsDir(list);
	int num_my_groups = 0;
	int add_size = 0;
	char *line = malloc(sizeof(char)*40);
	char *part_answer = calloc(40, sizeof(char));
	char *answer;
	char GID[30];

	if(num_groups == 0){ // if there doesn't exist any group
		printf("nao ha grupos\n");
		answer = malloc(sizeof(char)*7);
		sprintf(answer, "RGM 0\n"); // CHECK THIS
	}

	for(int i=0; i<num_groups; i++){
		if(CheckIfInGroup(list->group_no[i], token_list[1]) == 0){continue;} // senao fizer parte do grupo
		//printf("chegou a um grupo\n");
		strcpy(GID, list->group_no[i]);
		num_my_groups += 1;
		sprintf(line, " %s %s %s", GID, KnowGroupName(GID), KnowNumberMessagesInGroup(GID));
		add_size += 40; // adicionar tamanho que lemos
		strcat(part_answer, line);
		part_answer = realloc(part_answer, sizeof(char) * (add_size + 40));
	}

	if(num_my_groups == 0){ // if he's not in any group
		answer = malloc(sizeof(char)*7);
		sprintf(answer, "RGM 0\n");
	}
	else{
		answer = malloc(sizeof(char) * (add_size+40+10));
		sprintf(answer, "RGM %d%s\n", num_my_groups, part_answer);
        printf("Answer no mgl: %s", answer);

	}

	free(line);
	free(part_answer);
	return answer;
}



/**
 * command groups
*/
char* GroupsCommand(){
	GROUPLIST *list = malloc(sizeof(GROUPLIST));
	int num_groups = ListGroupsDir(list);
	char *end = "\n";
	if(num_groups == 0){ // if there aren't any groups
		char *answer = malloc(sizeof(char)*5);
		sprintf(answer, "RGL 0\n");
		return answer;
	}
	char *answer = calloc(sizeof(char)*(num_groups*60), sizeof(char));
	char *add_answer = calloc(sizeof(char)*60, sizeof(char));
	sprintf(answer, "RGL %d", num_groups);
	for(int i=0; i<num_groups; i++){
		sprintf(add_answer, " %s %s %s", list->group_no[i], list->group_name[i], KnowNumberMessagesInGroup(list->group_no[i]));
		strcat(answer, add_answer);
	}
	free(add_answer);
	strcat(answer, end);
	return answer;
}
