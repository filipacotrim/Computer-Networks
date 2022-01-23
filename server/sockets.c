// Server program
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "readmessages.h"
#include "commands.h"
#include "profunctions.h"
#include "validate.h"

#define MAXLINE 1024

int listenfd, connfd, udpfd, nready, maxfdp1, verbose;
pid_t childpid, pid;
fd_set rset;
ssize_t n;
socklen_t len;
const int on = 1;
struct sockaddr_in cliaddr, servaddr;
void sig_chld(int);



int max(int x, int y); // know the max
void initiatesockets(int portDS, int flag); // initiating the sockets
void handleTCPConnection(); // handles tcp connection
void handleUDPConnection(); // handles udp connection




int max(int x, int y){
    if (x > y){return x;}
    else{return y;}
}

void initiatesockets(int portDS, int flag){
    verbose = flag;
    /* create listening TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portDS);

    // binding server addr structure to listenfd
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 10); // nunca soube qual o segundo argumento

    /* create UDP socket */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // binding server addr structure to udp sockfd
    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // clear the descriptor set
    FD_ZERO(&rset);

    // get maxfd
    maxfdp1 = max(listenfd, udpfd) + 1;

    // programa principal
    for (;;) {

        // set listenfd and udpfd in readset
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        // select the ready descriptor
        nready = select(maxfdp1, &rset, NULL, NULL, NULL); // nao tem tempo le quando quer

        // if tcp socket is readable then handle
        // it by accepting the connection
        // JA ESTA FEITO
        if (FD_ISSET(listenfd, &rset)) {

            handleTCPConnection();
        }

        // if udp socket is readable receive the message.
        //ESTA PARTE TAMBEM VAI TER UM PROCESSO FIHO ASSOCIADO
        if (FD_ISSET(udpfd, &rset)) {

            handleUDPConnection();
        }
    }
}

void handleTCPConnection(){
    //printf("ACCEPTED: TCP\n");
    char clientIP[15];
    int clientPort;
    len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
    if ((childpid = fork()) == 0) { // criar um processo filho
        pid = getpid();
        close(listenfd); // fechar para o filho

        sprintf(clientIP, "%s", inet_ntoa(cliaddr.sin_addr));
        clientPort = (int) ntohs(cliaddr.sin_port);

        char data[512];
        char buffer[10];
        char *answer = calloc(12,sizeof(char));
        //limit of 300 registered uid's
        char *ulist_answer = calloc(3000,sizeof(char));
        char text2[240];
        char *op = calloc(4,sizeof(char));
        char *uid = calloc(5,sizeof(char));
        char mid[5];
        char *gid = calloc(2,sizeof(char));
        char *tsize = calloc(3,sizeof(char));
        char *text = calloc(240,sizeof(char));
        char *fname = calloc(24,sizeof(char));
        char *gname = calloc(24,sizeof(char));
        char *fsize = calloc(10,sizeof(char));
        int n, size;

        //reads opcode
        n = read(connfd,buffer,4);
        strcpy(op,buffer);
        op[n] = '\0';

        if(verbose){
            printf("-----------------------\n");
            printf("Request: %s\n\n", op);
            printf("From IP: %s and port: %d\n", clientIP, clientPort);                
            printf("-----------------------\n");
        }

        //printf("op: %s\n",op);
        if(!strcmp(op,"PST ")) { //POST
            memset(buffer,0,4);

            //reads UID
            n = read(connfd,buffer,5);
            strcpy(uid,buffer);
            memset(buffer,0,5);
            uid[n] = '\0';

            //printf("gid: %s\n",gid);

            //reads space
            n = read(connfd,buffer,1);
            memset(buffer,0,1);
            
            //reads GID
            n = read(connfd,buffer,2);
            strcpy(gid,buffer);
            memset(buffer,0,2);
            gid[n] = '\0';
            //printf("gid: %s\n",gid);

            int res = CheckIfGroupExists(gid,NULL);
            //printf("Res1: %d\n", res);
            if(res == 0) {
                write(connfd,"RPT NOK",7);
                close(connfd); // para o processo pai
                exit(1);
            }

            res = CheckIfInGroup(gid,uid);
            //printf("Res2: %d\n", res);

            if (res == 0) {
                write(connfd,"RPT NOK",7);
                close(connfd); // para o processo pai
                exit(1);
            }
    
            //reads space
            n = read(connfd,buffer,1);
            memset(buffer,0,1);

            //saves reads and saves Tsize
            int count = 0;
            while(count <= 3) { 
                read(connfd,buffer,1);
                //printf("buffer: %s\n",buffer);
                if(!strcmp(buffer," ")) {
                    size = atoi(tsize);
                    tsize[strlen(tsize)] = '\0';
                    //printf("/tsize: %d \n",size);
                    memset(buffer,0,1);
                    break;
                }
                else {
                    strcat(tsize,buffer);
                    count++;
                }
                memset(buffer,0,1);
            }

            //reads whole text
            n = read(connfd,text2,size);
            strcpy(text,text2);
            text[n] = '\0';
            memset(text2,0,size);

            //gets new message MID
            int MID = postGIDMSG(gid);
            sprintf(mid,"%04d",MID);
           
            //creates A U T H O R.txt
            createAuthortxt(uid,gid,mid);
            //creates T E X T.txt
            createTexttxt(uid,gid,mid,text);
            memset(text,0,size);
            
            //reads space
            n = read(connfd,buffer,1);

            buffer[n] = '\0';
            //checks if has file

            if (strcmp(buffer,"\n")) {
                memset(buffer,0,1);

                int count = 0;
                while(count <= 24) { 
                    //reads filename
                    read(connfd,buffer,1);
                    //printf("Buff: %s\n", buffer);
                    if(!strcmp(buffer," ")) {
                        fname[strlen(fname)] = '\0';
                        printf("/fname: %s\n",fname);
                        break;
                    }
                    else {
                        strcat(fname,buffer);
                        count++;
                    }
                    memset(buffer,0,1);
                }

                count = 0;
                while(count <= 10) { 
                    //reads file size
                    read(connfd,buffer,1);

                    if(!strcmp(buffer," ")) {
                        //size = atoi(fsize);
                        //fsize[strlen(fsize)] = '\0';
                        printf("/fsize: %s ",fsize);
                        break;
                    }
                    else {
                        strcat(fsize,buffer);
                        count++;
                    }
                    memset(buffer,0,1);
                }

                if(createFile(gid,mid,fname) == 1) {
                    char *pathname = malloc(sizeof(char)*50);

                    sprintf(pathname,"GROUPS/%s/MSG/%s/%s",gid,mid,fname);

                    FILE *file_pointer;
                    // use appropriate location if you are using MacOS or Linux
                    file_pointer = fopen(pathname,"w");
                    int size = atoi(fsize);
                    //printf("Fsize: %d\n", size);
                    int dim = 1;
                    int soma = 0;
                    while (size > 512 && dim != 0) {
                        dim = read(connfd,data,512);
                        fwrite(data,1,dim,file_pointer);
                        size -= dim;
                        soma += dim;
                        memset(data,0,512);
                    }
                    dim = read(connfd,data,size);
                    fwrite(data,1,size,file_pointer);
                    size -= dim;
                    soma += dim;
        	        //printf("Size inicial %s size final: %d Sum: %d\n",fsize, size, soma);
                    
                    //fwrite(data, 1, size, file_pointer);
                    memset(data,0,dim);
                    fclose(file_pointer);
                    free(fname);free(fsize);
                }  
            }
            sprintf(answer, "RPT %s\n", mid);
            write(connfd, answer, strlen(answer));
            free(gid); free(uid); free(text); free(tsize); free(answer);
            close(connfd);
            exit(0); 

        }else if(!strcmp(op,"ULS ")) {
            memset(buffer,0,4);

            //reads GID
            n = read(connfd,buffer,2);
            strcpy(gid,buffer);
            memset(buffer,0,2);
            gid[n] = '\0';

            //checks if group GID exists
            int res = CheckIfGroupExists(gid,NULL);
            if(res == 0) {
                write(connfd,"RUL NOK",7);
                close(connfd); // para o processo pai
                return;
            }
            //saves group name
            strcpy(gname,KnowGroupName(gid));

            //enters group gid directory
            char *pathname = malloc(sizeof(char)*60);
            sprintf(pathname, "GROUPS/%s", gid);

            sprintf(ulist_answer,"RUL OK %s",gname);

            DIR *d;
            struct dirent *dir;
        
            d = opendir(pathname); //opems directory
                
            if (d) {
                while ((dir = readdir(d)) != NULL) { 
                    //checks if is a file and if it is not the file containing group name (guid_name.txt)
                    if (dir->d_type == DT_REG && strlen(dir->d_name)<10) {
                        int size = strlen(dir->d_name);
                        strcat(ulist_answer," ");
                        dir->d_name[size-1] = '\0';
                        dir->d_name[size-2] = '\0';
                        dir->d_name[size-3] = '\0';
                        dir->d_name[size-4] = '\0';
                        strcat(ulist_answer,dir->d_name);
                    }
                }
            }

            write(connfd,ulist_answer,strlen(ulist_answer));
            printf("answer: %s\n",ulist_answer);
            close(connfd);
            free(ulist_answer); free(gid); free(gname);
            exit(0);
        } else if(!strcmp(op,"RTV ")){
            memset(buffer,0,4);

            //reads UID
            n = read(connfd,buffer,5);
            strcpy(uid,buffer);
            memset(buffer,0,5);
            uid[n] = '\0';

            //reads space
            n = read(connfd,buffer,1);
            memset(buffer,0,1);

            //reads GID
            n = read(connfd,buffer,2);
            strcpy(gid,buffer);
            memset(buffer,0,2);
            gid[n] = '\0';


            int res = CheckIfGroupExists(gid,NULL);
            if(res == 0) {
                write(connfd,"RRT NOK",7);
                close(connfd); // para o processo pai
                return;
            }

            //check if user is subscribed to group gid
            res = CheckIfInGroup(gid,uid);
            if (res == 0) {
                write(connfd,"RRT NOK",7);
                close(connfd); // para o processo pai
                return;
            }
    
            //reads space
            n = read(connfd,buffer,1);
            memset(buffer,0,1);

            //read mid
            n = read(connfd, buffer, 4);
            strncpy(mid, buffer, 4);

            //check if the msg exists and know how many we will retrieve
            res = knowNumberMessagesAfter(gid, mid);
            if(res == 0){
                write(connfd,"RRT EOF",7);
                close(connfd); // para o processo pai
                return;
            }

            if(res>20){res = 20;}

            sprintf(answer, "RRT OK %d ", res);
            write(connfd, answer, strlen(answer));
            memset(answer, 0, strlen(answer));

            for(int p = 0; p < res; p++){
                memset(uid, 0, 5);

                //writes author of message 
                strcpy(uid, KnowAuthorOfMessage(gid, mid));
                sprintf(answer, "%s %s ", mid, uid);

                write(connfd, answer, strlen(answer));
                memset(answer, 0, strlen(answer));

                memset(text,0,240);
                strcpy(text, KnowTextOfMessage(gid, mid));

                struct stat sb;

                char path[40];
                sprintf(path,"GROUPS/%s/MSG/%s/T E X T.txt",gid,mid);

                if(stat(path,&sb) == -1) {
                        perror("stat");
                        return;
                    }

                ssize_t tlen = sb.st_size;

                sprintf(answer, "%ld %s", tlen,text);
               
                write(connfd, answer, strlen(answer));
                memset(answer, 0, strlen(answer));

                int hasFile = checkIfAditionalFile(gid,mid);
                if(p ==(res - 1) && (hasFile == 2)) { //already read last message
                    write(connfd,"\n",1);
                    break;
                }
                else if (hasFile == 2) { //doesn't have file
                    int new_mid = atoi(mid) + 1;
                    sprintf(mid,"%04d",new_mid);
                    write(connfd," ",1);
                    continue;
                } else if (hasFile == 3) { //has file
                    write(connfd," / ",3);
                    strcpy(fname,KnowFileName(gid,mid));
                    write(connfd, fname, strlen(fname));
                    
                    char path[50];
                    sprintf(path,"GROUPS/%s/MSG/%s/%s",gid,mid,fname);
                   
                    write(connfd," ",1);

                    struct stat sb;

                    if(stat(path,&sb) == -1) {
                        perror("stat");
                        return;
                    }

                    ssize_t fsize = sb.st_size;

                    sprintf(answer,"%ld",fsize);
                    write(connfd,answer,strlen(answer));
                    write(connfd," ",1);

                    FILE* file_pointer;
                    file_pointer = fopen(path, "r");

                    int dim,soma = 0;
                    while(fsize > 512) {
                        dim = fread(data,sizeof(char),512,file_pointer);
                        if(send(connfd, data, dim, 0)==-1)
                            perror("send");
                        //memset(data,0,dim);
                        fsize -= dim;
                        soma += dim;
                        //printf("Fsize: %d\n",Fsize);
                    }
                    dim = fread(data,sizeof(char),fsize,file_pointer);
                    soma += dim;
                    send(connfd, data, dim, 0);
                    //printf("Fsize: %s Dim: %d\n", answer, soma);
                    
                    int new_mid = atoi(mid) + 1;
                    sprintf(mid,"%04d",new_mid);
                    //printf("New mid: %s\n", mid);
                    //write(connfd," ",1);
                    if(p ==(res - 1)){
                        write(connfd, "\n", 1);
                    }
                    else {
                        write(connfd," ",1);
                    }
                }

            }
            shutdown(connfd, SHUT_WR);
            int result = read(connfd, buffer, 1);
            if(result == 1){
                close(connfd);
            }
            free(uid); free(fsize); free(gid); free(fname); free(text);
            exit(0);
        }
    }
    close(connfd); // para o processo pai
    return;
}

void handleUDPConnection(){
    // NAO COLOCA O /N NO FIM
    //printf("ACCEPTED: UDP\n");
    // o pai caga so
    if ((childpid = fork()) == 0) { // filho faz as cenas maradas
        char *buffer = malloc(sizeof(char)*MAXLINE);
        char *answer;
        char clientIP[15];
        int clientPort;
        
        while(1){
            // unicas coisas que fez foi bzero do buffer
            // len = sizeof(cliaddr)
            len = sizeof(cliaddr);
            bzero(buffer, MAXLINE);
            n = recvfrom(udpfd, buffer, MAXLINE, 0,
                            (struct sockaddr*)&cliaddr, &len);

            sprintf(clientIP, "%s", inet_ntoa(cliaddr.sin_addr));
            clientPort = (int) ntohs(cliaddr.sin_port);
            if(verbose){
                printf("-----------------------\n");
                printf("Request: %s\n", buffer);
                printf("From IP: %s and port: %d\n", clientIP, clientPort);                
                printf("-----------------------\n");
            }

            answer = readCommands(buffer); // enviar para os comandos
            free(buffer); // ja nao precisamos dele

            sendto(udpfd, (char*)answer, strlen(answer), 0,
                (struct sockaddr*)&cliaddr, sizeof(cliaddr));

            free(answer); // isto da free ao do da answer
        }
    }

    return ;// processo pai nao precisa de fazer nada
}

