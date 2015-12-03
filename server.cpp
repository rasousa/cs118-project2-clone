//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
//Usage: ./server port cwnd (./server 10000 4)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>

#include "packet.h"

using namespace std;

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
    char response[PACKET_SIZE];
    
    if (argc < 3) {
       fprintf(stderr,"usage: port cwnd\n");
       exit(0);
    }
    
    portno = atoi(argv[1]);
    //cwnd = atoi(argv[2]);
    
    
    //Setup socket connection - SOCK_DGRAM is for UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        error("ERROR creating socket");
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    cout << "Server bound correctly on port "<< portno << endl;
    int count = 0;
    while(1)
    {
        //recvfrom dumps the message into packet
        if((recvfrom(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen)) < 0)
            error("ERROR receiving message");
        
        sprintf(response, "ack %d", count++);
        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&cli_addr, clilen) < 0)
            error("ERROR sending message");
        cout << packet << endl;
        
        
    }
    
}