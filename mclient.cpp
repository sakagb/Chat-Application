#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>

#define PORT 8888

void read_function(char buffer[], int sock)
{
	while(true)
	{
		// Wait for response
		memset(buffer, 0, 1024);
    	int bytesReceived = recv(sock, buffer, 1024, 0);

    	// Display response
    	std::cout << std::string(buffer, bytesReceived) << "\r\n";
	}
}

void write_function(std::string userInput, int sock)
{
	while(true)
	{
		// Enter lines of text
    	getline(std::cin, userInput);

    	// Send to server
    	int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
    	if (sendRes == -1)
    	{
        	std::cout << "Could not send to server! Whoops!\r\n";
    	}
	}
}

int main()
{
	char buffer[1024] = { 0 };
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	std::string userInput;
	
	// std::string hello;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	
	// Creates two parallel processes for receiving and sending messages
	std::thread th1(&read_function, buffer, sock);
	std::thread th2(&write_function, userInput, sock);

	if(th1.joinable())
		th1.join();
	if(th2.joinable())
		th2.join();

	// closing the connected socket
	close(client_fd);
	return 0;
}