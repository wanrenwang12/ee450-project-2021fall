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
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>
#include <climits>

using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define TCPPORTA 25809 // TCPPort
#define BUFLEN 1000


struct sockaddr_in centralAddrTCP;
int central_TCP_sockfd;
struct sockaddr_in clientAddr;
int client_sockfd;

char recv_message [BUFLEN];

void init_TCP(){
    // create socket
    //reference: Beej
    central_TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    centralAddrTCP.sin_family = AF_INET;
    centralAddrTCP.sin_port = htons(TCPPORTA);
    centralAddrTCP.sin_addr.s_addr = inet_addr(LOCALIP);

    // bind the socket
    if ( connect(central_TCP_sockfd, (struct sockaddr *) &centralAddrTCP, sizeof(centralAddrTCP)) == -1){
        perror("ClientA fails to connect to Central server");
        exit(EXIT_FAILURE);
    }
}

void sendToCentral(char* nameA){
    
    string buf;
    //send username1 to central server
    if (send(central_TCP_sockfd, nameA, strlen(nameA), 0) == -1){
        perror("ClientA fails to send nameA to Central server");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "The client sent " << nameA << " to the Central server" << endl;
    
}

void recvFromCentral(){

    int recvLen1;

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
        cout << "Please input in the following format: " << endl << "'./clientA <nameA>'" << endl;
        exit(EXIT_FAILURE);
    }

    init_TCP();
    cout << "The client is up and running." << endl;

    // set input into InputA
    char InputA[BUFLEN];
    sprintf(InputA,"%s",argv[1]);


    sendToCentral(InputA);
    recvFromCentral();

    cout << recv_message << endl;

    //clear and close
    memset(recv_message,0,sizeof(recv_message));
    close(client_sockfd);
    return 0;
}


