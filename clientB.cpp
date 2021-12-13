#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iomanip>
#include <ctype.h>
#include <vector>

using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define TCPPORTB 26809 // TCPPort
#define BUFLEN 1000


struct sockaddr_in centralAddrTCP;
int central_TCP_sockfd;
struct sockaddr_in clientAddr;
int client_sockfd;

char recv_message[BUFLEN];

//reference: Beej
void init_TCP(){
    // create the socket
    //reference: Beej
    central_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    centralAddrTCP.sin_family = AF_INET;
    centralAddrTCP.sin_port = htons(TCPPORTB);
    centralAddrTCP.sin_addr.s_addr = inet_addr(LOCALIP);

    // connect the socket
    if ( connect(central_TCP_sockfd, (struct sockaddr *) &centralAddrTCP, sizeof(centralAddrTCP)) == -1){
        perror("ClientB fails to connect to Central server");
        exit(EXIT_FAILURE);
    }

}

//reference: Beej
void sendToCentral(char* nameB){

    //send username2 to central server
    if (send(central_TCP_sockfd, nameB, strlen(nameB), 0) == -1){
        perror("ClientB fails to send nameB to Central server");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "The client sent " << nameB << " to the Central server" << endl;

}

void recvFromCentral(){

    int recvLen1;

    //reference:https://www.geeksforgeeks.org/socket-programming-cc/
    if ( (recvLen1 = recv(central_TCP_sockfd, recv_message, BUFLEN, 0)) == -1){
        perror("Fail to receive the message from Central");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char * argv[]) {

    //check the input format
    //reference: Beej
    if (argc != 2){
        cout << "Please input in the following format: " << endl << "'./clientB <nameB>' " << endl;
        exit(EXIT_FAILURE);
    }

    init_TCP();
    cout << "The client is up and running." << endl;

    //set input into InputB
    char InputB[BUFLEN];
    sprintf(InputB,"%s",argv[1]);

    sendToCentral(InputB);
    recvFromCentral();

    cout << recv_message << endl;

    //clear
    memset(recv_message,0,sizeof(recv_message));
    close(client_sockfd);
    return 0;
}