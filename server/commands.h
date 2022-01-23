#ifndef commands
#define commands


// UDP FUNCTIONS
int registerCommand(char *token_list[], int num_tokens); // command register
int unregisterCommand(char *token_list[], int num_tokens); // command unregister
int loginCommand(char *token_list[], int num_tokens); // command login
int logoutCommand(char *token_list[], int num_tokens); // command logout
int subscribeCommand(char *token_list[], int num_tokens); // command subscribe
int unsubscribeCommand(char *token_list[], int num_tokens); // command unsubscribe
char* my_groupsCommand(char *token_list[], int num_tokens); //command my_groups
char* GroupsCommand(); // command groups

// TCP FUNCTIONS
int listCommand(); // list command
int postCommand(char *token_list[],int num_tokens); // select command
int retrieveCommand(char *token_list[], int num_tokens); // retrieve command

#endif