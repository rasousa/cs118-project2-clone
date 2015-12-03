#define PACKET_SIZE 512

using namespace std;

enum PType { DATA, ACK };

struct Packet {
    PType type;
    int server_portno;
    int client_portno;
    int pid;
    int size;
    char data[PACKET_SIZE];
};