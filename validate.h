#ifndef VALIDATE_H
#define VALIDATE_H


int isAlpha(char* str, int flag); // checking if the str is alphanumeric
int isNumber(char* s); // checking if the str is a number
void checkingInput(int argc, char *argv[]); // first input and define port and IP
int isValidText(char *str); // checking if the text name is valid
int isValidFileName(char *str); // checking if the text

#endif