//server.cpp is what sends the packets to the client
//This file performs the Go-Back-N keeping track of the window
//Usage: ./receiver host port filename (./receiver 192.1.52.113 10000 hello.txt)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // define structures like hostent
#include <strings.h>
#include <string.h>
#include <unistd.h>

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
    float pl, pc;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t servlen = sizeof(serv_addr);
    string filename;
    struct hostent *server;
    
    if(argc != 6)
    {
        fprintf(stderr,"usage: server port filename pi pc\n");
        exit(0);
    }
    
    portno = atoi(argv[2]);
    filename = argv[3];
    pl = 100*atof(argv[4]);
    pc = 100*atof(argv[5]);
    
    server = gethostbyname(argv[1]);
    if(server == NULL)
    {
        error("Host not fount");
    }
                       
    
    
    //Setup socket connection - SOCK_DGRAM is for UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        error("ERROR creating socket");
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    memset((char *)&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(portno);
    
    
    cout << "Server bound correctly on port "<< portno << endl;
    
    int response_length;
    int seq_num = 0;
    int bytes_loaded = 0;
    int total_bytes = 0;
    Packet packet, ack, req;
    memset(&packet, 0, sizeof(packet));
    memset(&ack, 0, sizeof(ack));
    memset(&req, 0, sizeof(req));
    req.type = REQ;
    req.server_portno = ntohs(serv_addr.sin_port);
    req.client_portno = portno;
    req.seq_num = 0;
    req.size = filename.length();
    strcpy(req.data, filename.c_str());
    
    if (sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&serv_addr, servlen) < 0) {
        error("ERROR sending request");
    }
    
    ofstream file;
    file.open("testing.txt");
    
    srand((int)time(0));
    while(1)
    {
        //recvfrom dumps the message into packet
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &serv_addr, &servlen)) < 0)
            error("ERROR receiving message");
        
        //Basically here is an INIT packet that sends the file size and the filename in packet.data
        
        //print_packet(&packet);
        if(packet.type == INIT)
        {
            total_bytes = packet.size;
            bytes_loaded = 0;
            continue;
        }
        
        int plrand = rand()%100;
        int pcrand = rand()%100;
        
        if (pl > plrand)
        {
            cout << "Packet number: " << packet.seq_num << " lost." << endl;
            continue;
        }
        else if (pc > pcrand)
        {
            cout << "Packet number: " << packet.seq_num << " corrupted." << endl;
            memset(&ack, 0, sizeof(ack));
            ack.seq_num = seq_num;
            ack.type = ACK_CORRUPT;
            ack.size = bytes_loaded;
            ack.server_portno = ntohs(serv_addr.sin_port);
            ack.client_portno = portno;
            sprintf(ack.data, "ACK CORRUPT %d", seq_num);
            
            if (sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&serv_addr, servlen) < 0)
                error("ERROR sending ACK");
            continue;
        }
        
        if(packet.type == DATA)
        {
            if(seq_num == packet.seq_num)
            {
                
                bytes_loaded = (bytes_loaded >= total_bytes) ? total_bytes : bytes_loaded + packet.size;
                
                
                
                file << packet.data;
            
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
                seq_num++;
            }
            
        }
        
        //cout << total_bytes << " " << bytes_loaded << endl;
        
        if(total_bytes == bytes_loaded)
        {
            file.close();
            exit(0);
        }
        
    }
    
}