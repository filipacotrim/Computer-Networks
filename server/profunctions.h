#ifndef profunctions
#define profunctions


typedef struct groups{
	int no_groups; // numero de grupos
	char *group_name[101]; // nomes para cada grupo TENHO QUE LHES DAR O TAMANHO PARA PODER COPIAR
	char *group_no[25]; // para saber o GID
} GROUPLIST;

//sort
int sortstring( const void *str1, const void *str2 ); // function to sort string


int CheckIfUserExists(char *UID); // cheks if UID exists
int CreateUserDir(char *UID); // creates a user UID directory
int CreateUserPass(char *UID, char *PASS); // creates a file pass_txt for a UID
int DelUserDir(char *UID); // deletes a user UID directory
int CheckUserPass(char *UID, char *PASS); // checks if PASS is the password for UID
int CheckLoggedIn(char *UID); // checks if the UID is logged in
int CreateLoginUser(char *UID); // creates a uid_loggin file
int DeleteLoginUser(char *UID); // deletes a uid_loggin file
int DelPassFile(char *UID); // deletes a uid_pass file



int CheckIfGroupExists(char *GID, char *G_NAME); // know if the GID exists
int KnowHowManyGroups(); // know how many groups exist
char *KnowGroupName(char *GID); // know the group name of a given GID
int JoinGroup(char *GID, char *UID); // join a UID into group
int CreateGroup(char *GID, char *G_NAME, char *UID); // create the new group
int CheckIfInGroup(char *GID, char *UID); // know if the uid user is in the gid group
int RemoveUserFromGroup(char *GID, char *UID); // removes a user from a group
int ListGroupsDir(GROUPLIST *list);
int checkGID(char *GID);
int createFile(char *GID,char *MID,char *Fname);


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int CheckIfMessageExists(char *GID); // know if message exists within a group   ja esta
char* KnowNumberMessagesInGroup(char *GID); // know the number of last message of a given GID   ja esta
int knowNumberMessagesAfter(char *GID, char *MID); // Know how many messages come after     ja esta
char* KnowTheNewGid(int num_groups); // know the next GID   ja esta
char* KnowAuthorOfMessage(char *GID, char *MID); // know the author of a message
char* KnowTextOfMessage(char *GID, char *MID); // know the text of a message
int checkIfAditionalFile(char *GID, char *MID); // checks if has posted file
char* KnowFileName(char *GID, char *MID); //gets posted file name
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int createAuthortxt(char *UID, char *GID, char *MID);
int createTexttxt(char *UID, char *GID, char *MID, char *text);
int postGIDMSG(char *GID);

#endif