#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
	
#define TRUE 1
#define FALSE 0
#define PORT 8888

//extern int errno;

int start(int client_socket[4], int client_message[4], int begin[4], int max_clients, int warnings[4]){
	int contor = 0;
	for(int i = 0; i < max_clients; i++){
		if(client_socket[i] != 0 && client_message[i] != 0 && begin[i] == 1)
			contor++;
	}
	return contor;
}

void disconnectSocket(int poz, int client_message[4], int client_socket[4], int begin[4], int warnings[4]){
				
	//Close the socket and mark as 0 in list for reuse
	close(client_socket[poz]);
	//reset
	client_message[poz] = 0;
	begin[poz] = 0;
	client_socket[poz] = 0;
	warnings[poz] = 0;
}

int checkWinner(int connected_clients, int client_socket[4], int max_clients){
	if(connected_clients == 1) {
		for(int j = 0; j < max_clients; j++){
			if(client_socket[j] != 0){
				char sendMessage[1024];
				strcpy(sendMessage, "You won!\0");
				send(client_socket[j], sendMessage, strlen(sendMessage), 0);
				printf("\nGame ended. Socket %i won\n", j);
				close(client_socket[j]);
				return 1;
			}
		}
	}
	return 0;
}

int main(int argc , char *argv[])
{
	int opt = TRUE;
	int master_socket, addrlen, new_socket, max_clients, activity, i, valread, sd, max_sd;
	int client_message[4];//1 represents Jom, 2 represents Terry
	int client_socket[4];
	int warnings[4];
	int connected_clients;
	int begin[4];// begin[i] = 1 means the client i is ready to start the game
	int startGame;
	struct sockaddr_in address;
	char buffer[1025] = "";
	char caracter[1025] = "";
	char sendMessage[1024];
	int sendInitialMessage;
	struct timeval tv;

	//The game starts when all the connected clients wrote start

	//set of socket descriptors
	fd_set readfds;
		
	char *message = "Welcome";
	
	//Initialization
	//client_message[] = 0/1/2 : 0 if we don t have a client, 1 if the first connected client is Jom, 2 if the first connected client is Terry
	//initialise all client_socket[] to 0 so not checked and client_message[]  to 0, so no message
	connected_clients = 0;
	startGame = 0;
	max_clients = 4;
	sendInitialMessage = 1;
	for (int i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
		client_message[i] = 0;
		begin[i] = 0;
		warnings[i] = 0;
	}

	tv.tv_sec = 0;
	tv.tv_usec = 0;
		
	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
		
	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);
		
	//try to specify maximum of 4 pending connections for the master socket
	if (listen(master_socket, 4) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
		
	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");
	while(TRUE)
	{
		//clear the socket set
		FD_ZERO(&readfds);
		//add master socket to set
		FD_SET(master_socket, &readfds);
		//printf(master_socket);
		max_sd = master_socket;
			
		//add child sockets to set
		for ( i = 0 ; i < max_clients ; i++)
		{
			//socket descriptor
			sd = client_socket[i];
					
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
				
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
		if ((activity < 0) && (errno != EINTR)){
			printf("select error");
		}
				
		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			if(connected_clients < 4 && sendInitialMessage == 1){
				//inform user of socket number - used in send and receive commands
				printf("[Server]New connection: socket fd is %d , ip is : %s , port is : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		
				//send new connection greeting message
				if( send(new_socket, message, strlen(message), 0) != strlen(message) )
				{
					perror("send");
				}
					
				puts("Welcome message sent successfully");
					
				//add new socket to array of sockets
				for (i = 0; i < max_clients; i++)
				{
					//if position is empty
					if( client_socket[i] == 0 )
					{
						client_socket[i] = new_socket;
						connected_clients++;
						printf("Adding to list of sockets as %d\n" , i);
						break;
					}
				}
			}
			else{
				char sendMessage[1024];
				strcpy(sendMessage, "The server is closed");
				if(send(new_socket, sendMessage, strlen(sendMessage), 0) != strlen(sendMessage) )
				{
					perror("send");
				}
				close(new_socket);
			}
			
		}
		
		//else its some IO operation on some other socket
		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];
			startGame = start(client_socket, client_message, begin, max_clients, warnings);
			if (FD_ISSET(sd , &readfds))
			{
				//Check if it was for closing, and also read the
				//incoming message 
				valread = read(sd, buffer, 1024);
				
				if (valread <= 0){
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					printf("Client disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
					disconnectSocket(i, client_message, client_socket, begin, warnings);
					connected_clients--;
				}
				else if(valread > 0){
					
					printf ("\n[server]Client %i send :%s", sd, buffer);
					buffer[valread] = '\0';
					char buffer_copy[1025];
					bzero(buffer_copy, 1024);
					snprintf(buffer_copy, 1025, "%s", buffer);
					char *position = strstr(buffer_copy, "start");
					if(position != NULL && client_message[i] != 0)
						begin[i] = 1;
					if(buffer_copy[0] == '1' || buffer_copy[0] == '2'){
						client_message[i] = buffer_copy[0] - 48;//char to int
					}
					
				}
				
			}
			if(connected_clients == startGame && sd != 0){
				if(sendInitialMessage == 1){
					strcpy(sendMessage, "The game will start\0");
					printf("\nThe game will start");
					for(int j = 0; j < max_clients; j++){
						if(client_socket[j] != 0){	
							send(client_socket[j], sendMessage, strlen(sendMessage) , 0);
						}
					}
					sendInitialMessage = 0;
				}
				//printf("\nClient conectati %i.", connected_clients);
				if(checkWinner(connected_clients, client_socket, max_clients) == 1){
					return 0;
				}

				//Send warnings if the player doesn t own the magic ball
				for(int j = 0; j < max_clients; j++){
					if(sd != client_socket[j] && client_message[j] != 0 && client_message[i] != 0 && connected_clients > 0 ){
						//send the warning
						printf("\nSending warnings to the socket %i, from socket %i.", client_socket[j], sd);
						strcpy(sendMessage, "Warning\0");
						send(client_socket[j], sendMessage, strlen(sendMessage) , 0);
						
						warnings[j]++;
						printf("\nSocket %i, warnings received : %i.", client_socket[j], warnings[j]);
						if(warnings[j] == 3){
							//Somebody will be disconnected , get his details and print
							getpeername(client_socket[j], (struct sockaddr*)&address , (socklen_t*)&addrlen);
							printf("\nThe client %i was disconnected , ip %s , port %d \n",client_socket[j], inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
							disconnectSocket(j, client_message, client_socket, begin, warnings);
							connected_clients--;
							startGame = start(client_socket, client_message, begin, max_clients, warnings);
						}
						printf("\nConnected clients: %i", connected_clients);
						if(checkWinner(connected_clients, client_socket, max_clients) == 1){
							return 0;
						}
					}
					else if(sd == client_socket[j]){
						printf("\nSame socket");
					}	
					else{
						printf("\nNon-existent socket");
					}
				}
				sleep(3);
			}
		}
	}
		
	return 0;
}
