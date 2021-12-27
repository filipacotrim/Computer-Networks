#include <stdio.h> // input & output
#include <stdlib.h> // memory alocation, control processes
#include <string.h> // string operations
#include <unistd.h> // standard symbolic constants and types 
#include <errno.h> //defines the integer variable errno
#include <ctype.h> // testing and mapping characters
#include "sharedMacros.h" // macros shared by the entire program
#include "tcpandudp.h" // tcp and udp connections


// NO GLOBAL VARIABLES

// FUNCTIONS
int isAlpha(char* str, int flag); // checking if the str is alphanumeric
int isNumber(char* s); // checking if the str is a number
void checkingInput(int argc, char *argv[]); // first input and define port and IP
int isValidText(char *str); // checking if the text name is valid
int isValidFileName(char *str); // checking if the text
char readFromFile(char *Fname);




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
 * checks the first input (run input)
 * and, defines the ip and port for udp and tcp connections
 * 
 * input:
 *  + number parameters
 *  + list of parmeters
*/
void checkingInput(int argc, char *argv[]){
  char portDS[PORT_SIZE] = DEFAULT_DSPORT;
  char ipDS[IP_SIZE] = DEFAULT_DSIP;

    for (int i = 1; i < argc - 1; i++){
    if (argv[i][0] != '-') continue;
    switch (argv[i][1]){
            case 'n':
        // to prevent cases like ./user -n -p xxxx
        if (argv[i+1][0] != '-'){ strcpy(ipDS, argv[i+1]);}
        break;   

      case 'p':
        //to prevent cases like ./user -p -n xxxx
        if (argv[i+1][0] != '-'){ strcpy(portDS, argv[i+1]);}
        break;
      default:
        printf("Invalid flag.\nMust be on following format: ./user [-n DSIP] [-p DSport]\n");
        exit(1);
    }
  }

  portandIP(portDS, ipDS);

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
  if(isAlpha(token_list[0], 1) && strlen(token_list[0]) <= 24 
    && strlen(token_list[1])==3 && isAlpha(token_list[1], 1)){
      return 1;
  }
  return 0;
}


char readFromFile(char *Fname)
{
    FILE *fptr;
    char c;
    // Open file
    fptr = fopen(Fname, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    // Read contents from file
    c = fgetc(fptr);
    while (c != EOF)
    {
      printf ("%c", c);
      c = fgetc(fptr);
    }
    fclose(fptr);
    return c;
}
