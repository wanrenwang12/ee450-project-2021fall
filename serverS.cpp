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
#define UDPPORT 24809 // UDP Port For serverS
#define SERVERSPORT 22809 //UDP Port of S
#define BUFLEN 1000

char buf [BUFLEN];
char nameA [BUFLEN];
char nameB [BUFLEN];
int recvLen1;
int recvDone;
map<string, string> QueryResult; // result store as <string,string>

struct sockaddr_in centralAddrUDP;
struct sockaddr_in serverSAddr;
int serverS_sockfd;

void init_UDP(){
    // create socket
    //reference: Beej
    if ( (serverS_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Fail to create the UDP socket");
        exit(EXIT_FAILURE);
    }

    serverSAddr.sin_family = AF_INET;
    serverSAddr.sin_port = htons(SERVERSPORT);
    serverSAddr.sin_addr.s_addr = inet_addr(LOCALIP);

    // bind the socket
    if (::bind(serverS_sockfd, (struct sockaddr *) &serverSAddr, sizeof(serverSAddr)) == -1 ){
        close(serverS_sockfd);
        perror("Fail to bind the UDP socket");
        exit(EXIT_FAILURE);
    }
}

//recv topology and query socres, then send
void recv_And_Send(){

    // index_first, index_last seperate recv message into two names
    int index_first;
    int index_last;
    int sendLen;

    //0: not finished , 1: finished
    recvDone = 0;
    socklen_t centralLen = sizeof(centralAddrUDP);

    //sum_message used to piece the information together
    string sum_message;
    char revcmessage [BUFLEN];

    //    recv each edge contain two names
    if ((recvLen1 = recvfrom(serverS_sockfd, revcmessage, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) < 1){
        perror("Fail to receive the message from Central server");
        exit(EXIT_FAILURE);
    }

    //find the position of ',', which seperate two names and ';' :the end of the message
    for(int i = 0; i < BUFLEN; i++){
        if(revcmessage[i] == ','){
            index_first = i;
        }
        else if(revcmessage[i] == ';'){
            index_last = i;
            break;
        }
        //'.' represent the final message
        else if(revcmessage[i] == '.'){
            index_last = i;
            recvDone = 1;
            cout << "The ServerS received a request from Central to get the scores." << endl;
            break;
        }

    }

    //seperate two names and their scores according to map<name(key), score(value)>
    string first_name = string(revcmessage).substr(0, index_first);
    string second_name  = string(revcmessage).substr(index_first+1, index_last-index_first-1);
    string first_score = QueryResult[first_name];
    string second_score = QueryResult[second_name];

    /*
    cout << first_name << endl;
    cout << second_name << endl;
    cout << first_score << endl;
    cout << second_score << endl;
    */

    //make send message and format is :"nameA:score,nameB>score;" while the last message will end with "."
    if ( recvDone == 0){
        sum_message = first_name + ":" + first_score + "," + second_name + ">" + second_score + ";" ;
    }
    else{
        sum_message = first_name + ":" + first_score + "," + second_name + ">" + second_score + "." ;
        cout << "The ServerS finished sending the scores to Central." << endl;
    }

    char *send_message;
    send_message = (char*)sum_message.c_str();

    if ( ( sendLen = sendto(serverS_sockfd, send_message, strlen(send_message), 0, (struct sockaddr *) &centralAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to central from ServerS");
        exit(EXIT_FAILURE);
    }
    memset(send_message, '\0', strlen(send_message));

}

// store all information from scores.txt in a map < key, value>
void Query_Socres(){
    std::ifstream fileInput("scores.txt");
//    assert(fileInput.is_open()); // if open fail, abort
    string lineInFile;

    while(getline(fileInput, lineInFile)) {
        //get the position of space and seperate names and scores
        int index = lineInFile.find(' ');
        string name = lineInFile.substr(0, index);
        string score = lineInFile.substr(index + 1);

        QueryResult.insert(pair<string, string>(name, score));
    }
}




int main (){

    init_UDP();
    cout << "The ServerS is up and running using UDP on port " << SERVERSPORT << "." << endl;

    // init to make map<name, socre>
    Query_Socres();

    //revc , caculate and send to central until recvDone =1
    while(1){
        recv_And_Send();

        //clear
        memset(nameA, 0,sizeof(nameA));
        memset(nameB, 0, sizeof(nameB));
        memset(buf, 0, sizeof(buf));

    }

    return EXIT_SUCCESS;
}