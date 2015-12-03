
/*
 A simple client in the internet domain using UDP
 Usage: ./client hostname port filename (./client 192.168.0.151 10000 hello.html)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // define structures like hostent
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "packet.h"

using namespace std;

void error(string msg)
{
    cerr << msg << endl;
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd; //Socket descriptor
    int portno, n;
    string filename;
    struct sockaddr_in serv_addr;
    struct hostent *server; //contains tons of information, including the server's IP address
    socklen_t servlen = sizeof(serv_addr);

    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage: %s hostname port filename\n", argv[0]);
       exit(0);
    }
    
    portno = atoi(argv[2]);
    filename = argv[3];
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create a new socket
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    server = gethostbyname(argv[1]); //takes a string like "www.yahoo.com", and returns a struct hostent which contains information, as IP address, address type, the length of the addresses...
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //initialize server's address
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) //establish a connection to the server
        error("ERROR connecting");
        
    int response_length;
    int seq_num = 0;
    Packet req, packet, ack;
    memset(&req, 0, sizeof(req));
    memset(&packet, 0, sizeof(packet));
    memset(&ack, 0, sizeof(ack));
    
    //Create request packet for file
    req.type = REQ;
    req.server_portno = portno;
    req.client_portno = sockfd;
    req.seq_num = 0;
    req.size = filename.length();
    strcpy(req.data, filename.c_str());

		//Request file from server by sending the file name 
    //n = write(sockfd,filename.c_str(),filename.length());
    n = sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *) &serv_addr, servlen);
    if (n < 0) 
         error("ERROR writing to socket");
    
    //Wait to receive file from server
    while (1) 
    {
				//recvfrom dumps the message into packet
        if((response_length = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &serv_addr, &servlen)) < 0)
            error("ERROR receiving message");
        
        seq_num = packet.seq_num;
        
        cout << seq_num << endl;
        
        //PSEUDO CODE:
        //if packet is received
        //for seq = 0 start new file and write to it
        //for packet in order write to end of file
        //for out of order packet send ACK back with old seq_num
        //if end is received end write to file
    }
    
    close(sockfd); //close socket
    
    return 0;
}
