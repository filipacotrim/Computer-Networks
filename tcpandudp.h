#ifndef TCPANDUDP_H
#define TCPANDUDP_H


void portandIP(char *port, char *ip); // defines the ip adress and port 
void initializeUDP(); // initializes the udp connection with the server
void writeSocketTCP(char *msg,int fdDSTCP);
void sendMessageUDP(char *msg); // sending message in udp protocol
void sendMessageTCP(int fdDSTCP); // sending message in tcp protocol
void processResponseTCP(char *msg); // process the response given in tcp protocol
void processResponseUDP(char *msg); // process the response given in udp protocol
void endUDPconnection(); // ends udp connection with the server
int initializeTCP();

#endif