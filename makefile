all: client server
client : client.cpp
	g++ -o client client.cpp -I.
server : server.cpp
	g++ -o server server.cpp -I.
clean: 
	-rm *.o $(objects) client server