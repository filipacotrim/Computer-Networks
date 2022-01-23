#include <stdio.h> // input & output
#include <stdlib.h> // memory alocation, control processes
#include <string.h> // string operations
#include <unistd.h> // standard symbolic constants and types 
#include <errno.h> //defines the integer variable errno
#include <ctype.h> // testing and mapping characters
#include "sharedMacros.h" // macros shared by the entire program
#include "sockets.h"


// NO GLOBAL VARIABLES

// FUNCTIONS
void checkingInput(int argc, char *argv[]);
int isAlpha(char* str, int flag); // checking if the str is alphanumeric
int isNumber(char* s); // checking if the str is a number
int isValidText(char *str); // checking if the text name is valid
int isValidFileName(char *str); // checking if the text
char* readFromFile(char *Fname);
int getFileSize(char *Fname);
//char buffer1[128] = {0};



/**
 * checks the first input (run input)
 * and, defines the ip and port for udp and tcp connections
 * 
 * input:
 *  + number parameters
 *  + list of parmeters
*/
void checkingInput(int argc, char *argv[]){
  char portDS[PORT_SIZE] = "58061";
  int verbose = 0;

  for (int i = 1; i <= argc - 1; i++){
  if (argv[i][0] != '-') continue;
  switch (argv[i][1]){ 
    case 'v':
      verbose = 1;
      break;
    case 'p':
      //to prevent cases like ./user -p -n xxxx
      if (argv[i+1][0] != '-'){ strcpy(portDS, argv[i+1]);}
      break;
    default:
      printf("Invalid flag.\nMust be on following format: ./server [-p DSport] [-v]\n");
      exit(1);
    }
  }
  int port = atoi(portDS);
  initiatesockets(port, verbose);
}


/**
 * check if the string is a alphanumeric string
 * 
 * input:
 *   + str (string)
 *   + flag (different types of checking)
 *
 * output:
 *   + 1 (true)
 *   + 0 (false)
*/
int isAlpha(char* str, int flag) {
  int code;
  for (int i = 0; i < strlen(str); i++) {
    code = (int)str[i];
    if (flag == 0) {
      if(!isalnum(str[i])) {
        return 0;
      }
    }
    if (flag == 1) {
      //45 is ASCII code for "-" and 95 for "_"
      if(!isalnum(str[i]) && (code != 45) && (code != 95)) {
        return 0;
      }
    }
  }
  return 1;
}




/**
 * check if the string corresponds to a number
 * 
 * input:
 *  + str (string)
 * 
 * output:
 *  + 1 (valid)
 *  + 0 (invalid)
*/
int isNumber(char* str) {
  int len = strlen(str);
  for (int i = 0; i < len; i++) 
    if (!isdigit(str[i])){ return 0;}
  return 1;
}




/**
 * checking if the text is the valid
 * 
 * input:
 *  + str (text)
 * 
 * output:
 *  + 1 (valid)
 *  + 0 (invalid)
 */
int isValidText(char *str){
  int length = strlen(str);
  int first = (int)str[0];
  int last = (int)str[length-1];

  // checking the size, first and last characters
  if(length>242 || first!=34 || last!=34){ return 0;}
  return 1;
}



/**
 * checking if the filename given by the user is valid
 * 
 * input:
 *   + str (filename)
 *
 * ouput:
 *   + 1 (valid)
 *   + 0 (invalid)
 */
int isValidFileName(char *str){
  int num_tokens = 0;
  char *token_list[2];
  char* token = strtok(str, ".");
  token_list[num_tokens++] = token;
  token = strtok(NULL, " \n");
  token_list[num_tokens] = token;
  //printf("%s\n%s\n", token_list[0], token_list[1]);
  //MELHOR FAZER O CONTRARIO
  if(token_list[1] != NULL && isAlpha(token_list[0], 1) && strlen(token_list[0]) <= 24 
    && strlen(token_list[1])==3 && isAlpha(token_list[1], 1)){
      return 1;
  }
  return 0;
}


char* readFromFile(char *Fname)
{
    FILE *fptr;
    
    // Open file
    fptr = fopen(Fname, "rb");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }

    size_t fsize = getFileSize(Fname);
    char * buffer1 = (char*)malloc(sizeof(char)*(fsize + 128));

    fread(buffer1,fsize,1,fptr);
    //printf("BUffer: %s\n",buffer1);

    fclose(fptr);
    buffer1[fsize] = '\n';
    return buffer1;
    free(buffer1);
}

int getFileSize(char *Fname) {
    FILE *fptr;

    // Open file
    fptr = fopen(Fname, "rb");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }

    fseek(fptr, 0L, SEEK_END);
    int sz = ftell(fptr);
    //printf("Tamanho do ficheiro: %d\n", sz);

    fclose(fptr);
    return sz;
}