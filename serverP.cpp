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
#include <set>
#include <queue>
#include <list>
#include <sstream>
#include <cmath>
#include <string>

using namespace std;

#define LOCALIP "127.0.0.1" // IP Address of Host
#define UDPPORT 24809 // UDP Port of central server
#define SERVERPPORT 23809 //serverP's UDP port
#define BUFLEN 1000

//store inputs name from ClientA and B
string input1;
string input2;

//path is stored as <name, list>, list contains all path name from input1 to name
map<string, list<string>> path;

char buf [BUFLEN];
char nameA [BUFLEN];
char nameB [BUFLEN];
int recvLen1;

struct sockaddr_in centralAddrUDP;
struct sockaddr_in serverPAddr;
int serverP_sockfd;

// class for Edge, store node, neighbour node and their matching gap
//reference: https://blog.csdn.net/weixin_42375679/article/details/112466514
class Edge {
public:
    string node;
    double matching_gap;

    Edge(string neighbour_node){
        this->node = neighbour_node;
        this->matching_gap = 0;
    }

    Edge(string neighbour_node, double gap){
        this->node = neighbour_node;
        this->matching_gap = gap;
    }

    bool operator<(const Edge& obj) const {
        return obj.node > node;
    }

    bool operator==(const Edge& obj) const {
        return obj.node == node;
    }
};

//Class graph, store map as < name, set<Edge>>
//reference: https://blog.csdn.net/weixin_42375679/article/details/112466514
class Graph{
    map < string, set<Edge>> adj;

    // judge whether contains the node or not
    public:bool contains(string node){
        return adj.find(node) != adj.end();
    }

    // judge whether two nodes are neighbours
    public:bool isAdjacent(string node1, string node2){
        if(contains(node1) && contains(node2) && (node1 != node2)){
            for (auto edge : adj[node1])
                if(edge.node == node2)
                    return true;
        }
        return false;
    }

    public:void add_node(string node){
        if(!contains((node))){
            set<Edge> edge_list;
            adj[node] = edge_list;
        }
    }

    public:void add_edge(string node1, string node2, double gap){
        if(!isAdjacent(node1, node2)){
            adj[node1].insert(Edge(node2, gap));
            adj[node2].insert(Edge(node1, gap));
        }
    }

    public:vector<string> get_nodes(){
        vector<string> nodes;
        for(auto node: adj)
            nodes.push_back(node.first);
        return nodes;
    }


    // dihkstra algorithm, find the shortest path
    public:map<string, double> dijkstra(string start){

        //contains results of distance to start;
        map<string, double> distance;

        //a queue can sort automatically, accoring to distance, from shortest to longest
        priority_queue<pair<double, list<string>>, vector<pair<double, list<string>>>, greater<pair<double, list<string>>>> q;

        //init, for each node, set their distance as INT_MAX except start;
        for(string node: get_nodes()){
            if(node == start) distance[start] = 0;
            else distance[node] = INT_MAX;
        }

        set<string> visted;

        //start with list <0, start>
        list<string> temp_start;
        temp_start.push_front(start);
        q.push(make_pair(0, temp_start));

        while(!q.empty()){
            //pop the top factor, since q sort automaticlly, top one is the shortest one
            auto front = q.top();
            q.pop();

            // set u as the last factor: represent the path. ie.: start - Rachael - Oliver, Oliver represent the path
            string u = front.second.back();

            // if u is in visted , do nothing
            if(visted.find(u) != visted.end()) continue;
            else visted.insert(u);

            // set matching gap
            double shortest_dis = front.first;
            distance[u] = shortest_dis;

            // set path
            for(auto node : front.second)
                path[u].push_back(node);

            // for each neighbour of this shortest node, set their path through this shortest node
            for(auto v: adj[u]){
                if(visted.find(v.node) == visted.end()){

                    double distance_ToV = v.matching_gap;
                    list<string> temp_node;

                    // the neighbours' path is <start -shortest node's path - shortest node - neighbour node>
                    temp_node = path[u];
                    temp_node.push_back(v.node);
                    q.push(make_pair(shortest_dis + distance_ToV, temp_node));

                }
            }
        }
        return distance;

    }

    public:void clear(){
        adj.erase(adj.begin(), adj.end());
        path.clear();
    }

};

Graph g;


void init_UDP(){
    // create socket
    //reference: Beej
    if ( (serverP_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("Fail to create the UDP socket");
        exit(EXIT_FAILURE);
    }

    serverPAddr.sin_family = AF_INET;
    serverPAddr.sin_port = htons(SERVERPPORT);
    serverPAddr.sin_addr.s_addr = inet_addr(LOCALIP);

    // bind socket
    if (::bind(serverP_sockfd, (struct sockaddr *) &serverPAddr, sizeof(serverPAddr)) == -1 ){
        close(serverP_sockfd);
        perror("Fail to bind the UDP socket");
        exit(EXIT_FAILURE);
    }
}

// format transfer from string to int/double/float
//reference: https://stackoverflow.com/questions/16747915/c-converting-a-string-to-double
template <class Type>
Type stringToNum(const string& str)
{
    istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

//format transfer from double to string
//reference: https://stackoverflow.com/questions/1786497/sprintf-double-precision-in-c/1786866
string doubleToString(const double &dbNum)
{
    char *chCode;
    chCode = new(std::nothrow)char[20];
    sprintf(chCode, "%.2lf", dbNum);
    string strCode(chCode);
    delete []chCode;
    return strCode;
}


void recvFromCentral() {

    int recv_Done_P = 0;
    char temp[BUFLEN];

    //set 4 position index to seperate "nameA:score,nameB>score;"
    int index_One;
    int index_Two;
    int index_Three;
    int index_Four;

    socklen_t centralLen = sizeof(centralAddrUDP);

    //receive input1
    if ((recvLen1 = recvfrom(serverP_sockfd, nameA, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) < 1) {
        perror("Fail to receive from Central server");
        exit(EXIT_FAILURE);
    }
    input1 = string(nameA);

    //receive input2
    if ((recvLen1 = recvfrom(serverP_sockfd, nameB, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) < 1) {
        perror("Fail to receive from Central server");
        exit(EXIT_FAILURE);
    }
    input2 = string(nameB);

    //receive topology and scores. Format: name1:score,name2>score;
    while (!recv_Done_P) {
        if ((recvLen1 = recvfrom(serverP_sockfd, temp, BUFLEN, 0, (struct sockaddr *) &centralAddrUDP, &centralLen)) <
            1) {
            perror("Error receiving from Central server");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < BUFLEN; i++) {
            if (temp[i] == ':') {
                index_One = i;
            } else if (temp[i] == ',') {
                index_Two = i;
            } else if (temp[i] == '>') {
                index_Three = i;
            } else if (temp[i] == ';') {
                index_Four = i;
                break;
            }
                //'.' represent the final message
            else if (temp[i] == '.') {
                index_Four = i;
                recv_Done_P = 1;
                cout << "The ServerP received the topology and score information." << endl;
                break;
            }
        }

        //seperate the message according to the position index
        string node_One = string(temp).substr(0, index_One);
        string score_One = string(temp).substr(index_One+1, index_Two-index_One-1 );
        string node_Two = string(temp).substr(index_Two+1, index_Three-index_Two-1 );
        string score_Two = string(temp).substr(index_Three+1, index_Four-index_Three-1 );

        //add node to graph
        g.add_node(node_One);
        g.add_node(node_Two);

        //set value of edge
        double temp_One = stringToNum<double>(score_One);
        double temp_Two = stringToNum<double>(score_Two);
        double temp_Result = abs((temp_One - temp_Two)/(temp_One + temp_Two));

        // add edge
        g.add_edge(node_One, node_Two, temp_Result);
    }
}

void sendToCentral(char* nameA, char *nameB) {


    int sendLen;
    string temp_A;
    string temp_B;
    char* send_Message_A;
    char* send_Message_B;
    vector<string> nodeForB;

    //get matching gap
    map<string, double> Result = g.dijkstra(nameA);
    double temp_Dis = Result[nameB];

    //distance == INT_MAX means no compatibility
    if(temp_Dis == INT_MAX){
        temp_A = "Found no compatibility for " + string(nameA) + " and " + string(nameB) +".";
        temp_B = "Found no compatibility for " + string(nameB) + " and " + string(nameA) +".";
    }
    //distance = 0 and nameA != nameB means there exist name error
    else if(temp_Dis == 0 && nameA != nameB){
        temp_A = "Found no compatibility for " + string(nameA) + " and " + string(nameB) +".";
        temp_B = "Found no compatibility for " + string(nameB) + " and " + string(nameA) +".";
    }
    else{
        //set result for A and B when there exist path
        temp_A = "Found compatibility for " + string(nameA) + " and " + string(nameB) + ": " + "\n";
        for(auto node : path[string(nameB)]){
            temp_A += node;
            nodeForB.push_back(node);
            if(node != path[string(nameB)].back())
                temp_A += " --- ";
        }
        temp_A = temp_A + "\n" + "Compatibility score: " + doubleToString(temp_Dis);


        temp_B = "Found compatibility for " + string(nameB) + " and " + string(nameA) + ": " + "\n";
        for(int i = nodeForB.size()-1; i>=0; i-- ){
            temp_B += nodeForB[i];
            if(i != 0)
                temp_B += " --- ";
        }
        temp_B = temp_B + "\n" + "Compatibility score: " + doubleToString(temp_Dis);


    }
    send_Message_A = (char*)temp_A.c_str();
    send_Message_B = (char*)temp_B.c_str();

    if ((sendLen = sendto(serverP_sockfd, send_Message_A, strlen(send_Message_A), 0, (struct sockaddr *) &centralAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the UDP message to central from ServerP");
        exit(EXIT_FAILURE);
    }

    if ((sendLen = sendto(serverP_sockfd, send_Message_B, strlen(send_Message_B), 0, (struct sockaddr *) &centralAddrUDP, sizeof(struct sockaddr_in))) == -1) {
        perror("Fail to send the  UDP message to central from ServerP");
        exit(EXIT_FAILURE);
    }
    Result.clear();
    cout << "The ServerP finished sending the results to the Central." << endl;

}

int main (){

    init_UDP();
    cout << "The ServerP is up and running using UDP on port " << SERVERPPORT << "." << endl;
    while(1){
        recvFromCentral();
        sendToCentral(nameA, nameB);

        //clear
        g.clear();
        memset(nameA, 0,sizeof(nameA));
        memset(nameB, 0, sizeof(nameB));
        memset(buf, 0, sizeof(buf));

    }

    return EXIT_SUCCESS;
}