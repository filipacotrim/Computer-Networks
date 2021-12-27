#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "sharedMacros.h"
#include "readCommands.h"
#include "validate.h"
#include "tcpandudp.h"


/** 
  Main function
  */
int main(int argc, char *argv[]){
  checkingInput(argc, argv); // checking input from the user 
  initializeUDP(); // initialize udp socket 
  readCommands(); // read commands from the user
}

