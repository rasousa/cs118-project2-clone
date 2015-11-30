//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>

using namespace std;

const int PACKET_SIZE = 100;

void error(string msg)
{
    cerr << msg << endl;
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd, portno, cwnd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(serv_addr);
    
    char packet[PACKET_SIZE];
    
    portno = atoi(argv[1]);
    cwnd = atoi(argv[2]);
    
    
    //Setup socket connection - SOCK_DGRAM is for UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        error("ERROR creating socket");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    while(1)
    {
        //recvfrom dumps the message into packet
        if((recvfrom(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen)) < 0)
            error("ERROR receiving message");
    }
    
}