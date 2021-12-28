#ifndef COMMANDS_H
#define COMMANDS_H

// FUNCTIONS
int registerCommand(char *token_list[], int num_tokens); // command register
int unregisterCommand(char *token_list[], int num_tokens); // command unregister
int loginCommand(char *token_list[], int num_tokens); // command login
int logoutCommand(char *token_list[], int num_tokens);
int subscribeCommand(char *token_list[], int num_tokens); // command subscribe
int unsubscribeCommand(char *token_list[]); // unsubscribe command
int showuidCommand(char *token_list[], int num_tokens); // showuid command
int my_groupsCommand(); //my_gorups command
int selectCommand(char *token_list[]); // select command
int listCommand(); // list command
int postCommand(char *token_list[],int num_tokens); // select command
int retrieveCommand(char *token_list[], int num_tokens); // retrieve command

#endif