#ifndef FILEFUNCTIONS_H
#define FILEFUNCTIONS_H

int CheckIfFileExists(char *pathname); // checks if file exists
int CheckIfDirExists(char *pathname); // check if directory exists
int CreateFile(char *pathname); // creates a file
int DeleteFile(char *pathname); // deletes a file
int CreateDirectory(char *pathname); // creates a directory
int DeleteDirectory(char *pathname); // deletes a directory
int DeleteAllFilesFromDirectory(char* dir_name); // deletes all files in a directory

#endif