all: sender receiver
sender : sender.cpp
	g++ -o sender sender.cpp -I.
receiver : receiver.cpp
	g++ -o receiver receiver.cpp -I.
clean: 
	-rm *.o $(objects) sender receiver