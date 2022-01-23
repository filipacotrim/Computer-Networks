#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "filefunctions.h"

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



char* KnowNumberMessagesInGroup(char *GID); // know the number of last message of a given GID
char* KnowTheNewGid(int num_groups); // know the next GID
int createAuthortxt(char *UID, char *GID, char *MID);
int createTexttxt(char *UID, char *GID, char *MID, char *text);
int postGIDMSG(char *GID);





/**
 * function to sort strings
*/
int sortstring( const void *str1, const void *str2 )
{
    char *const *pp1 = str1;
    char *const *pp2 = str2;
    return strcmp(*pp1, *pp2);
}


/**
 * 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 *
 * user
 *
 * 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
*/

/**
 * checks if a user exists
 *
 * output:
 *  + 1 (yes)
 *  + 0 (no)
*/
int CheckIfUserExists(char *UID){
	char user_dirname[20];
	sprintf(user_dirname, "USERS/%s", UID);
	return CheckIfDirExists(user_dirname);
}



/**
 * creates a user directory
 * it also checks for duplicates
 *
 * ouput
 *  + 1 (ok)
 *  + 2 (dup)
 *  + 0 (error)
 */
int CreateUserDir(char *UID)
{
	char user_dirname[20];
	sprintf(user_dirname,"USERS/%s",UID);
	return CreateDirectory(user_dirname);
}



/**
 * creates the pass file
 *
 * output:
 *  + 1 (ok)
 *  + 0 (error)
*/
int CreateUserPass(char *UID, char *PASS)
{
	FILE *file_pointer;
	char *pass = malloc(sizeof(char) * (strlen(PASS)+1));
	char *filename = malloc(sizeof(char) * (2*strlen(UID)+17));
	sprintf(filename, "USERS/%s/%s_pass.txt", UID, UID);
	sprintf(pass, "%s\n", PASS);

	if(CreateFile(filename) == 0) return 0; // erro a criar o ficheiro

	file_pointer = fopen(filename, "w");
	fputs(pass, file_pointer); // colocar a pass
	fclose(file_pointer);

	free(filename);
	free(pass);
	return 1; // all good
}


/**
 * checks if a user is logged in, also checks if user exists
 *
 * ouput:
 * 	+ 1 (yes)
 *  + 0 (no)
*/
int CheckLoggedIn(char *UID){
	char *pathname = malloc(sizeof(char)*(18+2*strlen(UID)));
	sprintf(pathname, "USERS/%s/%s_login.txt", UID, UID);
	return CheckIfFileExists(pathname);
}



/**
 * creates the login file
 *
 * output:
 *  + 1 (ok)
 *  + 0 (error)
 */
int CreateLoginUser(char *UID){
	char *filename = malloc(sizeof(char) * (17+2*strlen(UID)));
	sprintf(filename, "USERS/%s/%s_login.txt", UID, UID);

	if(CreateFile(filename) == 0){ printf("nao criou o ficheiro"); return 0;} // error creating file

	free(filename);
	return 1;
}


/**
 * delete the login file of a UID user
 *
 * output:
 *  + 1 (deleted)
 *  + 0 (error)
*/
int DeleteLoginUser(char *UID){
	char *filename = malloc(sizeof(char) * (17+2*strlen(UID)));
	sprintf(filename, "USERS/%s/%s_login.txt", UID, UID);
	return DeleteFile(filename);
}



/**
 * deletes a user directory and all it's files
 * it also checks if UID didn't exist
 */
int DelUserDir(char *UID)
{
	char user_dirname[20];
	sprintf(user_dirname,"USERS/%s",UID);
	DeleteLoginUser(UID);
	if(DelPassFile(UID) == 0) return 0;
	return DeleteDirectory(user_dirname); // deleting the directory
}


/**
 * checks if the pass is the right for the user
 * also, checks if the user exists
 *
 * output:
 *  + 1 (yes)
 *  + 0 (no)
*/
int CheckUserPass(char *UID, char* PASS){
	FILE *fileptr;
	char *pass = malloc(sizeof(char)*9);
	char *filename = malloc(sizeof(char) * (20+2*strlen(UID)));
	sprintf(filename, "USERS/%s/%s_pass.txt", UID, UID);

	if(CheckIfUserExists(UID) == 0) return 0;

	fileptr = fopen(filename, "r");
	if(fileptr == NULL){printf("error opening the file\n"); return 0;}

	fgets(pass, 9, fileptr);
	if(strcmp(pass, PASS)) // if they are different
	{
		free(filename);
		free(pass);
		return 0;
	}

	free(pass);
	free(filename);
	return 1; // all good
}



/**
 * deletes the password of the given uid
 *
 * output:
 *  + 1 (deleted)
 *  + 0 (error)
 */
int DelPassFile(char *UID)
{
	char pathname[50];
	sprintf(pathname,"USERS/%s/%s_pass.txt",UID,UID);
	return DeleteFile(pathname);
}




/**
 * check if a groups exists (gid,  gname)
 * but can also work to check only gid (gid, NULL)
 *
 * output:
 *  + 1 (yes)
 *  + 2 (different name)
 *  + 0 (no)
*/
int CheckIfGroupExists(char *GID, char *G_NAME)
{
	FILE *file_pointer;
	char *name_in_file = calloc(24, sizeof(char));
	char *group_pathname = malloc(sizeof(char)*33);
	char *gname_pathname = malloc(sizeof(char)*60);
	sprintf(group_pathname, "GROUPS/%s", GID);
	sprintf(gname_pathname, "GROUPS/%s/%s_name.txt", GID, GID);
	if( CheckIfDirExists(group_pathname) == 0) {return 0;} // group didn't exist

	if(G_NAME == NULL) // if we only were curious about the number and not the name
	{
		free(group_pathname);
		free(gname_pathname);
		return 1;
	}

	file_pointer = fopen(gname_pathname, "r");
	if(file_pointer == NULL){printf("error opening the file\n"); return 0;}

	fgets(name_in_file, 24, file_pointer);
	if(strcmp(name_in_file, G_NAME)) // if they are different
	{
		free(group_pathname);
		free(gname_pathname);
		return 2;
	}
	free(group_pathname);
	free(gname_pathname);
	return 1;
}



/**
 * know how many groups exist
 *
 * output:
 *  + number
 *  + -1 (error)
*/
int KnowHowManyGroups(){
	DIR *d;
	struct dirent *dir;
	int i=0, number = 0;
	d = opendir("GROUPS");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(dir->d_name[0]=='.'){continue;} // nao sei porque
			if(strlen(dir->d_name)>2){continue;} // se nao for um nome de um grupo

			// se tivermos a ler uma diretoria
			if(dir->d_type == DT_DIR){number++;}
			else{continue;} // se for um ficheiro em vez de

			if(i==99){break;}
		}
		closedir(d);
	}
	else
	{
		return(-1);
	}
	return number;
}



/**
 * know the group GID name
 * GID ALREADY CONFIRMED
 *
 * output:
 * 	+ name
*/
char* KnowGroupName(char *GID)
{
	// sacar o nome do grupo GROUPS/GID/GID_name.txt
	FILE *file_pointer;
	char *name = malloc(sizeof(char)*25);
	char *gname_pathname = malloc(sizeof(char)*60);
	sprintf(gname_pathname, "GROUPS/%s/%s_name.txt", GID, GID); // pathname for name of the group
	file_pointer = fopen(gname_pathname, "r");
	if(file_pointer == NULL){printf("error opening the file\n"); return 0;} // error opening the file
	fgets(name, 25, file_pointer);
	return name;
}



/**
 * know how many messages are in the group
 * GID ALREADY CONFIRMED
 *
 * output:
 * 	+ number of messages
*/
char* KnowNumberMessagesInGroup(char *GID)
{
	int number_messages = 0;
	char* answer = (char*)malloc(sizeof(char)*6);
	DIR * dir_pointer;
	struct dirent * entry;
	char *pathname = malloc(sizeof(char)*30);
	sprintf(pathname, "GROUPS/%s/MSG", GID);
	dir_pointer = opendir(pathname);
	while ((entry = readdir(dir_pointer)) != NULL) {
		if(entry->d_name[0]=='.'){continue;} 
		// if it's a directory and it's lenght name is 2
    	if (entry->d_type == DT_DIR) {
         	number_messages++;
    	}
	}
	closedir(dir_pointer);
	//printf("number msg: %d\n",number_messages);
	if(number_messages == 0){
		sprintf(answer, "0000");
	}
	else{
		if(number_messages < 10){
			sprintf(answer, "000%d", number_messages);
		}else{
			sprintf(answer, "00%d", number_messages);
		}
	}
	return answer;
}


/**
 * know the new GID
 *
 * output:
 * 	+ char* gid (created)
 *  + null (max)
*/
char* KnowTheNewGid(int num_groups){
	int new_number = num_groups +1;
	char *new_gid = malloc(sizeof(char)*2);
	if( new_number < 10)
	{
		sprintf(new_gid, "0%d", new_number);
	}
	else if(new_number > 99){
		return NULL;
	}
	else{
		sprintf(new_gid, "%d", new_number);
	}
	return new_gid;
}



/**
 * know if message exists
 * CHECKS IF GID EXISTS
 *
 * output:
 * 	+ 1 (yes)
 *  + 0 (no)
*/
int CheckIfMessageExists(char *GID)
{
	if(CheckIfGroupExists(GID, NULL) == 0) return 0;

	DIR *dir_pointer;
	char* pathname = malloc(sizeof(char)*30);
	sprintf(pathname, "GROUPS/%s/MSG", GID);
	dir_pointer = opendir(pathname);
	//printf("a diretoria onde estou a verificar: %s\n", pathname);
	if(dir_pointer == NULL){free(pathname);return 0;} // if we couldn't open the directory
	free(pathname);
	return 1; // all good
}



/**
 * know the number of messages that come after a MID
 *
 * output:
 * 	+ number (number of messages)
*/
int knowNumberMessagesAfter(char *GID, char *MID)
{
	if(CheckIfMessageExists(GID) == 0) return 0; // if the message doesn't exist
	char* number_msg;
	int number_messages, mid, result;
	number_msg = KnowNumberMessagesInGroup(GID);
	number_messages = atoi(number_msg);
	//printf("numero de mensagens no grupo: %d\n", number_messages);
	mid = atoi(MID);
	result = number_messages - mid +1;
	//printf("numero de mensagens para a frente: %d\n", result);
	return result;
}



/**
 * knowing the author of the message
 * CHECKS IF GROUP EXISTS
 *
 * output:
 * 	+ author
 *  + null (error)
*/
char* KnowAuthorOfMessage(char *GID, char *MID)
{
	if(CheckIfGroupExists(GID, NULL) == 0) return NULL;
	if(CheckIfMessageExists(GID) == 0) return NULL;

	FILE *file_pointer;
	char *pathname = malloc(sizeof(char)*40);
	char *author = malloc(sizeof(char) *5);
	sprintf(pathname, "GROUPS/%s/MSG/%s/A U T H O R.txt", GID, MID);
	//printf("estou a tentar abrir o ficheiro: %s\n", pathname);
	file_pointer = fopen(pathname, "r");
	if(file_pointer == NULL) {free(pathname); free(author);return NULL;} // error
	fread(author,1, 5, file_pointer);
	//printf("o autor era: %s\n", author);
	free(pathname);
	return author;
}



/**
 * know the text of message
 *
 * output:
 * 	+ text
 *  + NULL (error)
*/
char* KnowTextOfMessage(char *GID, char *MID)
{
	if(CheckIfGroupExists(GID, NULL) == 0) return NULL;
	if(CheckIfMessageExists(GID) == 0) return NULL;

	FILE *file_pointer;
	char pathname[40];
	char *text = malloc(sizeof(char) *240);
	sprintf(pathname, "GROUPS/%s/MSG/%s/T E X T.txt",GID,MID);
	//printf("estou a tentar abrir o ficheiro: %s\n", pathname);
	file_pointer = fopen(pathname, "r");
	if(file_pointer == NULL) {return NULL;}
	fread(text,1, 240, file_pointer);
	//printf("text: %s\n", text);
	return text;
	free(text);
}

int checkIfAditionalFile(char *GID, char *MID)
{
	char pathname[40];
	sprintf(pathname, "GROUPS/%s/MSG/%s",GID,MID);
	//printf("pathname: %s\n",pathname);

	DIR * dir_pointer;
	struct dirent * entry;
	dir_pointer = opendir(pathname);
	int count = 0;
	//checks files in MSG, if theres more than 2 it means there's a posted file
	while ((entry = readdir(dir_pointer)) != NULL) {
		if((entry->d_name[0]=='.') || !strncmp(entry->d_name,"..",2)) continue;
		count++;
	}
	//printf("count : %d\n",count);
	return count;
	closedir(dir_pointer);
}

char* KnowFileName(char *GID, char *MID)
{
	FILE *file_pointer;
	char pathname[40];
	char *fname = malloc(sizeof(char) *24);
	sprintf(pathname, "GROUPS/%s/MSG/%s",GID,MID);
	
	file_pointer = fopen(pathname, "r");
	if(file_pointer == NULL) {return NULL;}

	DIR * dir_pointer;
	struct dirent * entry;
	dir_pointer = opendir(pathname);

	//checks files in MSG, if theres more than 2 it means there's a posted file
	while ((entry = readdir(dir_pointer)) != NULL) {
		if (strcmp(entry->d_name,"A U T H O R.txt") && strcmp(entry->d_name,"T E X T.txt")
		           && entry->d_name[0]!='.' && strncmp(entry->d_name,"..",2)) {
			strcpy(fname,entry->d_name);
			return fname;
		}
	}
	return 0;
	closedir(dir_pointer);
}

/**
 * UID joins group GID
 *
 * output:
 * 	+ 1 (yes)
 *  + 0 (error)
*/
int JoinGroup(char *GID, char *UID)
{
	int res;
	char *pathname = malloc(sizeof(char)*50);
	sprintf(pathname, "GROUPS/%s/%s.txt", GID, UID);
	res = CreateFile(pathname);
	free(pathname);
	if(res == 0) return 0; // error and dup
	else return 1; // all good
}



/**
 * create new group
 *
 * output:
 * 	+ 1 (created)
 *  + 0 (error)
*/
int CreateGroup(char *GID, char *G_NAME, char *UID){
	char *gid_pathname = malloc(sizeof(char)*50);
	char *gid_gname_pathname = malloc(sizeof(char)*60);
	char *messages_gid_pathname = malloc(sizeof(char)*60);
	char *user_gid_pathname = malloc(sizeof(char)*60);

	sprintf(gid_pathname, "GROUPS/%s", GID);
	sprintf(gid_gname_pathname, "GROUPS/%s/%s_name.txt", GID, GID);
	sprintf(messages_gid_pathname, "GROUPS/%s/MSG", GID);
	sprintf(user_gid_pathname, "GROUPS/%s/%s.txt", GID, UID);

	if(CreateDirectory(gid_pathname) == 0) return 0; // create Directory Gid
	if(CreateFile(gid_gname_pathname) == 0) return 0; // create file name
	if(CreateDirectory(messages_gid_pathname) == 0) return 0; // create MSG directory
	if(CreateFile(user_gid_pathname) == 0) return 0; // create user file in GId directory

	FILE *file_pointer;
	// use appropriate location if you are using MacOS or Linux
   	file_pointer = fopen(gid_gname_pathname,"w");

   	if(file_pointer == NULL){return 0;}

   	fprintf(file_pointer, "%s", G_NAME);
   	fclose(file_pointer);
	return 1; // all good
}




/**
 * checks if a user is in a group
 * checks if groups exists
 *
 * output:
 * 	+ 1 (yes)
 *  + 0 (no or error)
*/
int CheckIfInGroup(char *GID, char *UID)
{
	char *pathname = malloc(sizeof(char) * (60));
	sprintf(pathname, "GROUPS/%s/%s.txt", GID, UID);

	if(CheckIfGroupExists(GID, NULL) == 0) {free(pathname);return 0;}
	//if(CheckLoggedIn(UID) == 0) {free(pathname);return 0;}
	if(CheckIfFileExists(pathname) == 0) {free(pathname);return 0;}
	free(pathname);
	return 1; // all good
}



/**
 * removes a uid from a group
 * DOESN'T check if group and uid valid
 *
 * output:
 *  + 1 (deleted)
 *  + 0 (error)
*/
int RemoveUserFromGroup(char *GID, char *UID)
{
	char *pathname = malloc(sizeof(char)*60);
	sprintf(pathname, "GROUPS/%s/%s.txt", GID, UID);
	if(DeleteFile(pathname) == 0){free(pathname); return 0;}
	return 1; // all good
}




/**
 * lista todos os grupos existentes na base de dados, fornece também o nome dos grupos
 *
 * TENHO QUE FAZER A ESTRUTURA
 *
 * ouput:
 *  + numero de grupos
 **/
int ListGroupsDir(GROUPLIST *list){
	DIR *d;
	struct dirent *dir;
	int i=0;
	FILE *fp;
	char GIDpathname[1000];
	list->no_groups=0;
	d = opendir("GROUPS");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if(dir->d_name[0]=='.'){continue;} // nao sei porque
			if(strlen(dir->d_name)>2){continue;} // se nao for um nome de um grupo

			list->group_no[i] = malloc(sizeof(char)*3);
			list->group_name[i] = malloc(sizeof(char)*25);
			strcpy(list->group_no[i], dir->d_name); // copiar para o group_no o numero do grupo
			sprintf(GIDpathname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
			fp=fopen(GIDpathname,"r");
			if(fp)
			{
				++i;
				/*
				list->group_name[i] = malloc(sizeof(char)*25);
				fscanf(fp,"%24s",list->group_name[i]);
				fclose(fp);
				*/
			}
			if(i==99){break;}
		}
		list->no_groups = i;
		closedir(d);
	}
	else
	{
		return(-1);
	}

	if(list->no_groups > 1)
	{
		qsort(list->group_no, i, sizeof(char*), sortstring); // sorting the list of gids
	}

	// putting the names in the list
	for(int j = 0; j<i; j++){
		strcpy(list->group_name[j], KnowGroupName(list->group_no[j]));
	}

	return(list->no_groups); // retorna o numero de grupos
}

int postGIDMSG(char *GID) {
	DIR *d;
	struct dirent *dir;
	char *bar = "/";
	int mid;
	char *groupdir = malloc(sizeof(char) * (strlen("GROUPS")+2*strlen(bar)+strlen(GID)+strlen("MSG")));
	sprintf(groupdir, "GROUPS/%s/MSG", GID);
	//printf("groupdir: %s\n",groupdir);
	d = opendir(groupdir);

	if (d) {
		mid = 0;
		//searches for directory in GROUPS with name gid
		while ((dir = readdir(d)) != NULL) {
			mid++;
		}
		mid--;
		//printf("mid: %d\n",mid);
		//ads new one
		char new_mid[5];
		sprintf(new_mid,"%04d",mid);
		char *new = malloc(sizeof(char) * (strlen(groupdir)+strlen(bar)+strlen(new_mid)));
		sprintf(new,"%s/%s",groupdir,new_mid);

		mkdir(new,0700);
		closedir(d);
		return mid;
		free(new);
	}
	return 0;
	free(groupdir);
}

int createAuthortxt(char *UID, char *GID, char *MID)
{
	FILE *fileptr;
	char *filename = malloc(sizeof(char) * (strlen("GROUPS")+strlen("A U T H O R.txt")+
	4+strlen(GID)+strlen(MID)));
	sprintf(filename, "GROUPS/%s/MSG/%s/A U T H O R.txt", GID, MID);
	//printf("filename: %s\n",filename);
	char *uid = malloc(sizeof(char) * (strlen(UID)+1));
	sprintf(uid, "%s\n", UID);

	// if there is already a user with that name
	fileptr = fopen(filename, "w");

	fputs(uid, fileptr); // colocar a pass

	fclose(fileptr);
	free(filename);
	free(uid);
	return 1; // all good
}

int createTexttxt(char *UID, char *GID, char *MID, char *text)
{
	FILE *fileptr;
	char *filename = malloc(sizeof(char) * (strlen("GROUPS")+strlen("T E X T.txt")+
	4+strlen(GID)+strlen(text)));
	sprintf(filename, "GROUPS/%s/MSG/%s/T E X T.txt", GID, MID);
	//printf("filename: %s\n",filename);
	char *data = malloc(sizeof(char) * (strlen(text)+1));
	sprintf(data, "%s\n", text);

	// if there is already a user with that name
	fileptr = fopen(filename, "w");

	fputs(data, fileptr); // colocar a pass

	fclose(fileptr);
	free(filename);
	free(data);
	return 1; // all good
}

int checkGID(char *GID){
	FILE *fileptr;

	char gid[9];

	sprintf(gid, "GROUPS/%s", GID);
	//printf("gid: %s\n",gid);

	fileptr = fopen(gid, "r");
	if(fileptr == NULL) // if the user doesn't exist
	{
		memset(gid,0,9);
		return 0;
	}

	memset(gid,0,9);
	return 1; // all good
}

int createFile(char *GID,char *MID,char *Fname) {
	char *pathname = malloc(sizeof(char)*50);

	sprintf(pathname,"GROUPS/%s/MSG/%s/%s",GID,MID,Fname);

	FILE *file_pointer;
	// use appropriate location if you are using MacOS or Linux
   	file_pointer = fopen(pathname,"w");

   	if(file_pointer == NULL){return 0;}

	return 1; // all good
}