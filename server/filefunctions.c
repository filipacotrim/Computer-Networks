#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <errno.h>


/**
 * check if a file exists
 *
 * output:
 * 	+ 1 (yes)
 *  + 0 (no)
*/
int CheckIfFileExists(char *pathname){
	if(access(pathname, F_OK) == 0) return 1;
	else return 0;
}


/**
 * check if directory exists
 *
 * output:
 * 	+ 1 (yes)
 *  + 0 (no)
*/
int CheckIfDirExists(char *pathname){
	int errnum;
	DIR* dir = opendir(pathname);
	if (dir) {
    	closedir(dir);
		return 1;
	} else{
		errnum = errno;
		printf("errnum: %d\n", errnum);
		perror("error: ");
		return 0;
	}
}


/**
 * create a file, given a pathname
 *
 * ouput:
 *  + 1 (ok)
 *  + 2 (already exists)
 *  + 0 (error)
*/
int CreateFile(char *pathname){
	if(CheckIfFileExists(pathname) == 1){ return 2;}
	FILE* file_pointer;
	file_pointer = fopen(pathname, "w");
	if(file_pointer != NULL) // if created
	{
		fclose(file_pointer);
		return 1;
	}
	else // if not created
	{
		fclose(file_pointer);
		return 0;
	}
}


/**
 * deletes a given filename
 *
 * output:
 *  + 1 (deleted)
 *  + 0 (error)
*/
int DeleteFile(char *pathname){
	if(CheckIfFileExists(pathname) != 1){ return 0;} // if it doesn't exist
	if(unlink(pathname)!=0){
		return 0;
	}
	return 1;
}



/**
 * creates directory given by pathname
 * also checks for duplicates
 *
 * ouput:
 * 	+ 1 (created)
 *  + 2 (dup)
 *  + 0 (error)
*/
int CreateDirectory(char *pathname){
	int ret, errnum;
	ret = mkdir(pathname, 0700);
	if(ret == -1)
	{
		errnum = errno;
		if(errnum == 17){ return 2; } // dup
		else return 0; // other error
	}
	return 1; // all good
}


/**
 * delete directory with given pathname
 *
 * output:
 *  + 1 (deleted)
 *  + 0 (error)
*/
int DeleteDirectory(char *pathname){
	int errnum;
	char cmd[11];
	sprintf(cmd, "rmdir %s", pathname);
	printf("dir: %s\n",cmd);
	int result = system(cmd);
	if(result!=0) // error
	{
		errnum = errno;
		perror("error: \n");
		if(errnum == 2)
		{
			printf("nao havia essa diretoria.\n");
		}
		return 0;
	}
	//printf("na teoria apagou");
	return 1; // deleted
}



/**
 * deletes all files from a directory
 *
 * DOESN'T CHECK IF DIRECTORY EXIST
 * output:
 *  + 1 (all deleted)
 *  + 0 (error)
*/
int DeleteAllFilesFromDirectory(char* dir_name){
	DIR *d;
	struct dirent *dir;
	char *file_pathname = malloc(sizeof(char)*50);
	fprintf(stdout, "pathname: %s", dir_name); // isto nao ta a aparecer
	d = opendir(dir_name);

	if (d)
	{
		printf("abriu a diretoria\n");
		while ((dir = readdir(d)) != NULL)
		{
			if(dir->d_name[0]=='.'){continue;} // nao sei porque
			if(strlen(dir->d_name)>2){continue;} // se nao for um nome de um grupo

			// se tivermos a ler um ficheiro
			if(dir->d_type == DT_REG){
				sprintf(file_pathname, "%s/%s.txt", dir_name, dir->d_name);
				printf("vamos apagar o ficheiro: %s\n", file_pathname);
				if(DeleteFile(file_pathname) == 0){ return 0;}
			}
			else{continue;} // se for uma diretoria ou algo diferente
		}
		closedir(d);
	} else{
		return 0;
	}
	return 1;
}
