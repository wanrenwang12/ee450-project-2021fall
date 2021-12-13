#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <iomanip>
#include <math.h>

using namespace std;


#define LOCALIP "127.0.0.1" // IP Address of Host
#define TCPPORTA 25809 // TCP Port for clientA
#define TCPPORTB 26809 // TCP Port for clientB
#define UDPPORT 24809 // UDP Port for 3 server T,S,P
#define SERVERTPORT 21809  //UDP Port of T
#define SERVERSPORT 22809  // UDP Port of S
#define SERVERPPORT 23809  // UDP Port of P
#define BACKLOG 3 // backlog of pending connections for listen
#define BUFLEN 1000

char bufForA [BUFLEN];// final result from P and send to A
char bufForB [BUFLEN];// final result from P and send to B
int recvLen1, sendLen;

char nameA [BUFLEN];// buffer for input from clientA
char nameB [BUFLEN];// buffer for input from clientB


int recv_Done_T = 0;
int recv_Done_S;

//reference: Beej
struct sockaddr_in centralAddrTCPA, centralAddrTCPB, centralAddrUDP, clientAAddr, clientBAddr, serverTAddr, serverSAddr, serverPAddr;
int central_TCP_sockfd, central_UDP_sockfd, A_central_TCP_sockfd, B_central_TCP_sockfd, new_A_central_TCP_sockfd, new_B_central_TCP_sockfd;


void init_TCPA(){
    
       //reference: Beej
       if ( (A_central_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
           perror("Fail to create the TCP socket");
           exit(EXIT_FAILURE);
       }

       centralAddrTCPA.sin_family = AF_INET;
       centralAddrTCPA.sin_port = htons(TCPPORTA);
       centralAddrTCPA.sin_addr.s_addr = inet_addr(LOCALIP);
       
       //Bind socket
       if (::bind(A_central_TCP_sockfd, (struct sockaddr *) &centralAddrTCPA, sizeof(centralAddrTCPA)) == -1 ){
           close(A_central_TCP_sockfd);
           perror("Fail to create the TCP socket");
           exit(EXIT_FAILURE);
       }
    //reference:https://www.geeksforgeeks.org/socket-programming-cc/
    if (listen(A_central_TCP_sockfd, BACKLOG) == -1){
        perror("Fail to listen");
        exit(EXIT_FAILURE);
    }
}

void init_TCPB(){

    //reference:Beej
    if ( (B_central_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Fail to create the TCP socket");
        exit(EXIT_FAILURE);
    }
    centralAddrTCPB.sin_family = AF_INET;
    centralAddrTCPB.sin_port = htons(TCPPORTB);
    centralAddrTCPB.sin_addr.s_addr = inet_addr(LOCALIP);

    //bind socket for clientB
    if (::bind(B_central_TCP_sockfd, (struct sockaddr *) &centralAddrTCPB, sizeof(centralAddrTCPB)) == -1 ){
        close(B_central_TCP_sockfd);
        perror("Fail to bind the TCP socket");
        exit(EXIT_FAILURE);
    }

    if (listen(B_central_TCP_sockfd, BACKLOG) == -1){
        perror("Fail to listen");
        exit(EXIT_FAILURE);
    }
}

void init_UDP(){
    // create the UDP socket for three compute servers
    if ( (central_UDP_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Fail to create UDP socket");
        exit(EXIT_FAILURE);
    }
    
    centralAddrUDP.sin_family = AF_INET;
    centralAddrUDP.sin_port = htons(UDPPORT);
    centralAddrUDP.sin_addr.s_addr = inet_addr(LOCALIP);

    //bind socket
    if (::bind(central_UDP_sockfd, (struct sockaddr *) &centralAddrUDP, sizeof(centralAddrUDP)) == -1 ){
        close(central_UDP_sockfd);
        perror("Fail to bind the UDP socket");
        exit(EXIT_FAILURE);
    }
}

void acceptFromClientA(){
    socklen_t clientLen = sizeof(clientAAddr);
    //reference:Beej
    if ( (new_A_central_TCP_sockfd = accept(A_central_TCP_sockfd,(struct sockaddr *) &clientAAddr, &clientLen)) == -1){
        perror("Fail to accept the socket");
        exit(EXIT_FAILURE);
    }
}

void acceptFromClientB(){
    socklen_t clientLen = sizeof(clientBAddr);

    if ( (new_B_central_TCP_sockfd = accept(B_central_TCP_sockfd,(struct sockaddr *) &clientBAddr, &clientLen)) == -1){
        perror("Fail to accept the socket");
        exit(EXIT_FAILURE);
    }


}

int recvFromClientA() {

    recvLen1 = recv(new_A_central_TCP_sockfd, nameA, BUFLEN, 0);

    if (recvLen1 == -1) {
        perror("Fail to receive the message from clientA");
        close(A_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    nameA[recvLen1] = '\0';// add the end to char array

    cout << "The Central server received input = \""<< nameA <<"\" from the client using TCP over port " << TCPPORTA << "." << endl;
    //return 1, then main function can judge whether receive
    return 1;
}

int recvFromClientB() {

    recvLen1 = recv(new_B_central_TCP_sockfd, nameB, BUFLEN, 0);
    if (recvLen1 == -1) {
        perror("Fail to receive the message from clientB");
        close(B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    nameB[recvLen1] = '\0';

    cout << "The Central server received input = \""<< nameB <<"\" from the client using TCP over port " << TCPPORTB << "." << endl;
    //return 1, then main function can judge whether receive
    return 1;
}


// Set port and IP of three servers.
void setServerTSP(){
    //    Server T
    serverTAddr.sin_family = AF_INET;
    serverTAddr.sin_port = htons(SERVERTPORT);
    serverTAddr.sin_addr.s_addr = inet_addr(LOCALIP);
    
    //    Server S
    serverSAddr.sin_family = AF_INET;
    serverSAddr.sin_port = htons(SERVERSPORT);
    serverSAddr.sin_addr.s_addr = inet_addr(LOCALIP);

    //   Server P
    serverPAddr.sin_family = AF_INET;
    serverPAddr.sin_port = htons(SERVERPPORT);
    serverPAddr.sin_addr.s_addr = inet_addr(LOCALIP);
}

// Send two inputs to serverT
void sendToT(){
    
    if ((sendLen = sendto(central_UDP_sockfd, nameA, strlen(nameA), 0, (struct sockaddr *) &serverTAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to Server T from Central");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    if ((sendLen = sendto(central_UDP_sockfd, nameB, strlen(nameB), 0, (struct sockaddr *) &serverTAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to Server T from Central");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    
    cout << "The Central server sent a request to Backend-Server T. " << endl;
}

// receive topology and send the topology to serverS
void recvFromT_sendToS(){

    char sendToS [BUFLEN];
    socklen_t serverTLen = sizeof(serverTAddr);

    // receive each message from T and store them in sendToS temporarily
    recvLen1 = recvfrom(central_UDP_sockfd, sendToS, BUFLEN, 0, (struct sockaddr *) &serverTAddr, &serverTLen );
    if (recvLen1 < 0){
        perror("Fail to receive the message from Server T");
        close(A_central_TCP_sockfd);
        close(B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }

    // receive '.' means has received all information
    if (sendToS[recvLen1-1] == '.'){
        recv_Done_T = 1;
        cout << "The Central server received information from Backend-server T using UDP over port "<< UDPPORT << "." << endl;
    }

    // send the message in sendToS to S
    if ((sendLen = sendto(central_UDP_sockfd, sendToS, strlen(sendToS), 0, (struct sockaddr *) &serverSAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to Server S from Central");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }

    // clear sendToS
    memset(sendToS, '\0', sizeof(sendToS));
}

//receive the scores from S, send the topology and scores to P
void recvFromS_sendToP(){

    //temporarily store received message, then send
    char temp[BUFLEN];
    socklen_t serverSLen = sizeof(serverSAddr);

    //send two input names to P
    if ((sendLen = sendto(central_UDP_sockfd, nameA, strlen(nameA), 0, (struct sockaddr *) &serverPAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to Server P from Central");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    if ((sendLen = sendto(central_UDP_sockfd, nameB, strlen(nameB), 0, (struct sockaddr *) &serverPAddr, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to Server P from Central");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }

    recv_Done_S = 0; // 0:receive not finished, 1:finished
    while(recv_Done_S == 0){
        if ((recvLen1 = recvfrom(central_UDP_sockfd, temp, BUFLEN, 0, (struct sockaddr *) &serverSAddr, &serverSLen )) < 0){
            perror("Fail to receive the message from Server S");
            close(new_A_central_TCP_sockfd);
            close(new_B_central_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        temp[recvLen1] = '\0';// add end to send message.
//        cout << temp << endl;

        // if temp end with '.' , means its the last message, set recv_Done_S as 1;
        if(temp[recvLen1-1] == '.'){
            recv_Done_S = 1;
            cout << "The Central server received information from Backend-server S using UDP over port "<< UDPPORT << "." << endl;
        }

        if ((sendLen = sendto(central_UDP_sockfd, temp, BUFLEN, 0, (struct sockaddr *) &serverPAddr, sizeof(struct sockaddr_in))) == -1) {
            perror("Fail to send the UDP message to Server P from Central");
            close(new_A_central_TCP_sockfd);
            close(new_B_central_TCP_sockfd);
            exit(EXIT_FAILURE);
        }
        memset(temp, '\0', sizeof(temp));//clear

    }
    cout << "The Central server sent a processing request to Backend-Server P. " << endl;

}


//receive the message from P to A and B, store in  bufForA, bufForB.
void recvFromP(){

    socklen_t serverPLen = sizeof(serverPAddr);

    //result for A
    if ((recvLen1 = recvfrom(central_UDP_sockfd, bufForA, BUFLEN, 0, (struct sockaddr *) &serverPAddr, &serverPLen )) < 0){
        perror("Fail to receive the message from Server P");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    bufForA[recvLen1] = '\0';//add end

    //result for B
    if ((recvLen1 = recvfrom(central_UDP_sockfd, bufForB, BUFLEN, 0, (struct sockaddr *) &serverPAddr, &serverPLen )) < 0){
        perror("Fail to receive the message from Server P");
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    bufForB[recvLen1] = '\0';//add end

    cout << "The Central server received the results from backend server P." << endl;
}


//send bufForA, bufForB to clientA, clientB.
void sendToClient(){

    if (send(new_A_central_TCP_sockfd, bufForA, BUFLEN, 0) == -1){
        perror("Fail to send the result to clientA");
        close(new_A_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    cout << "The Central server sent the results to client A." << endl;

    if (send(new_B_central_TCP_sockfd, bufForB, BUFLEN, 0) == -1){
        perror("Fail to send the result to clientB");
        close(new_A_central_TCP_sockfd);
        exit(EXIT_FAILURE);
    }
    cout << "The Central server sent the results to client B." << endl;

}





int main (){
    
    
    
    // initialize new port
    init_TCPA();
    init_TCPB();
    init_UDP();

    cout << "The Central server is up and running." << endl;

    //set port and address for each server
    setServerTSP();

    while (1) {

        // 0:clientA or B not send a request, 1: A or B send
        int a_receive = 0;
        int b_receive = 0;

        acceptFromClientA();
        acceptFromClientB();

        //if receive from A and B, set a_receive, b_receive as 1, and enter loop
        if(recvFromClientA()){a_receive = 1;}
        if(recvFromClientB()){b_receive = 1;}

        while((a_receive)&&(b_receive)){

            sendToT();

            //if !recv_Done_T, continue recvFromT_sendToS
            while(!recv_Done_T){
                recvFromT_sendToS();
            }

            cout << "The Central server sent a request to Backend-Server S. " << endl;
            //clear, make next request work
            recv_Done_T = 0;

            recvFromS_sendToP();

            recvFromP();

            sendToClient();
            a_receive = 0;
            b_receive = 0;

        }
        //clear
        memset(nameA, 0,sizeof(nameA));
        memset(nameB, 0, sizeof(nameB));
        memset(bufForA, 0, sizeof(bufForA));
        memset(bufForB, 0, sizeof(bufForB));

        //close TCP socket for A and B
        close(new_A_central_TCP_sockfd);
        close(new_B_central_TCP_sockfd);
    }

    return EXIT_SUCCESS;
}
