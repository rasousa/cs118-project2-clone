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
#include <fstream>

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
    socklen_t servlen = sizeof(serv_addr);
    
    
    portno = atoi(argv[1]);
    if(argc == 3)
        cwnd = atoi(argv[2]);
    
    
    //Setup socket connection - SOCK_DGRAM is for UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        error("ERROR creating socket");
    memset((char *)&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = INADDR_ANY;
    cli_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
        error("ERROR on binding");
    
    cout << "Server bound correctly on port "<< portno << endl;
    
    int response_length;
    int seq_num = 0;
    int bytes_loaded = 0;
    int total_bytes = 0;
    Packet packet, ack;
    memset(&packet, 0, sizeof(packet));
    memset(&ack, 0, sizeof(ack));
    ofstream file;
    
    while(1)
    {
				//recvfrom dumps the message into packet
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &serv_addr, &servlen)) < 0)
            error("ERROR receiving message");
        
        
        //Basically here is an INIT packet that sends the file size and the filename in packet.data
        if(packet.type == INIT)
        {
            file.open(packet.data);
            total_bytes = packet.size;
            bytes_loaded = 0;
        }
        
        if(packet.type == DATA)
        {
            if(seq_num == packet.seq_num)
            {
                
                bytes_loaded = (bytes_loaded >= total_bytes) ? total_bytes : bytes_loaded + packet.size;
                
                seq_num++;
                
                file << packet.data;
            }
            
            //Let's send an ack pack. If it's out of order it will send a duplicate.
            memset(&ack, 0, sizeof(ack));
            ack.seq_num = seq_num;
            ack.type = ACK;
            ack.size = bytes_loaded;
            ack.server_portno = ntohs(serv_addr.sin_port);
            ack.client_portno = portno;
            sprintf(ack.data, "ACK %d", seq_num);
            
            cout << ack.data << endl;
            if (sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&serv_addr, servlen) < 0)
                error("ERROR sending ACK");
        }
        
        if(total_bytes == bytes_loaded)
        {
            file.close();
        }

        
    }
    
}