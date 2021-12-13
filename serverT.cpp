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
#define UDPPORT 24809 //central's UDP port
#define SERVERTPORT 21809//serverT's UDP port
#define BUFLEN 1000

char buf [BUFLEN];
char nameA [BUFLEN];
char nameB [BUFLEN];
//char *sendmessage;
int recvLen1;

// path: store all path from edgelist.txt,  relatedpath :store all related path
vector< pair <string, string> > path;
vector< pair <string, string> > relatedpath;

// isPush: 0 represent path[i] not push, 1 represent push
int* isPush;

struct sockaddr_in centralAddrUDP;
struct sockaddr_in serverTAddr;
int serverT_sockfd;

void init_UDP(){

    //reference: Beej
    //create the socket
    if ( (serverT_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Fail to create the UDP socket");
        exit(EXIT_FAILURE);
    }

    serverTAddr.sin_family = AF_INET;
    serverTAddr.sin_port = htons(SERVERTPORT);
    serverTAddr.sin_addr.s_addr = inet_addr(LOCALIP);
    
    // bind the socket
    if (::bind(serverT_sockfd, (struct sockaddr *) &serverTAddr, sizeof(serverTAddr)) == -1 ){
        close(serverT_sockfd);
        perror("Fail to bind the UDP socket");
        exit(EXIT_FAILURE);
    }
}

void recvFromCentral(){

    socklen_t centralLen = sizeof(centralAddrUDP);
    //    recv input1
    if ((recvLen1 = recvfrom(serverT_sockfd, nameA, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) < 1){
        perror("Fail to receive A from Central server");
        exit(EXIT_FAILURE);
    }
    
    //    recv input2
    if ((recvLen1 = recvfrom(serverT_sockfd, nameB, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) < 1){
        perror("Fail to receive B from Central server");
        exit(EXIT_FAILURE);
    }
    
    cout << "The ServerT received a request from Central to get the topology." << endl;
}

//read edgelist.txt and construct all path.
void constructPath(){
    //open file and exit if fail to open the file
    std::ifstream fileInput("edgelist.txt");
    string lineInFile;
    while(getline(fileInput, lineInFile)){
        //get the position of space and seperate each line into two factors
        int index = lineInFile.find(' ');
        string first = lineInFile.substr(0, index);
        string second = lineInFile.substr(index + 1);

        //add all pair factors into path
        pair<string, string> each_line;
        each_line = make_pair(first, second);
        path.push_back(each_line);

        // initailize all isPush as 0
        isPush = new int[path.size()];
        for (int j=0; j < path.size(); j++ ){
            isPush[j] = 0;
        }
    }
}

//according to path, construct all related path
void construct_relatedPath(string vectorA, string vectorB ){

    // push the path that is equal to <vectorA, vectorB>
    for(int i = 0; i < path.size(); i++){
        if((!isPush[i]) && (((path[i].first == vectorA) && (path[i].second == vectorB)) || ((path[i].second == vectorA) && (path[i].first == vectorB)))) {
            relatedpath.push_back(path[i]);
            isPush[i] = 1;  // set isPush as 1
        }
    }

    // push a path that one factor of the path is equal to one of the input
    for(int i = 0; i < path.size(); i++){
        if( (!isPush[i]) && (((path[i].first == vectorA) ^ (path[i].second == vectorB)) || ((path[i].first == vectorB) ^ (path[i].second == vectorA)))) {
            relatedpath.push_back(path[i]);
            isPush[i] = 1;
            construct_relatedPath(path[i].first, path[i].second);//iteration
        }
    }
}


void sendToCentral(){

    int sendLen;

    // send each row of result except the last row, and end with ";"
    for(int i = 0; i < relatedpath.size()-1; i++){
        char *sendmessage;
        string temp = relatedpath[i].first + "," + relatedpath[i].second + ";";
        sendmessage = (char*)temp.c_str();
        if ( ( sendLen = sendto(serverT_sockfd, sendmessage, strlen(sendmessage), 0, (struct sockaddr *) &centralAddrUDP, sizeof(struct sockaddr_in))) == -1) {
            perror("Fail to send the UDP message to central from ServerT");
            exit(EXIT_FAILURE);
        }
    }

    // send the last row, and end with "."
    string last_temp = relatedpath[relatedpath.size()-1].first + "," + relatedpath[relatedpath.size()-1].second + ".";
    char *last_message;
    last_message = (char*)last_temp.c_str();
    if ( ( sendLen = sendto(serverT_sockfd, last_message, strlen(last_message), 0, (struct sockaddr *) &centralAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to central from ServerT");
        exit(EXIT_FAILURE);
    }
    
    cout << "The ServerT finished sending the topology to Central." << endl;

}

int main (){
    
    init_UDP();
    cout << "The ServerT is up and running using UDP on port " << SERVERTPORT << "." << endl;

    while(1){
        recvFromCentral();

        string first_vector = string(nameA);
        string second_vector = string(nameB);

        constructPath();
        construct_relatedPath(first_vector, second_vector);

        sendToCentral();

        memset(nameA, 0,sizeof(nameA));
        memset(nameB, 0, sizeof(nameB));
        memset(buf, 0, sizeof(buf));

    }
    return EXIT_SUCCESS;
}




