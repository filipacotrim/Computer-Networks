#ifndef SOCKETS_H
#define SOCKETS_H


int max(int x, int y); // know the max
void initiatesockets(int portDS, int flag); // initiating the sockets
void handleTCPConnection(); // handles tcp connection
void handleUDPConnection(); // handles udp connection

#endif
