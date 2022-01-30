#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX 4
#define PORT 8888
#define SA struct sockaddr

/* codul de eroare returnat de anumite apeluri */
//extern int errno;

void checkNumber(char msg[1024], int sockfd){
	int wrong_input = 1;
	while(wrong_input){//as long as the message read is different from 1 or 2, I will return an error 
	    bzero (msg, 1024);
        printf ("\n[client]Introduce 1 to play with Jom, introduce 2 to play with Terry: ");
        fflush (stdout);
        read (0, msg, 1024);
		char send_message[1024];
		//Jom = Terry = 0;
		char msg_copy[1025];
		snprintf(msg_copy, 1024, "%s", msg);
		char *position1 = strchr(msg_copy, '1');
		char *position2 = strchr(msg_copy, '2');
		if(position1 != NULL){
			printf("\n[Client]You will play with Jom.");
			strcpy(send_message, "1\0");
			wrong_input = 0;
		}
		else if(position2 != NULL){ 
				printf("\n[Client]You will play with Terry");
				strcpy(send_message, "2\0");
				wrong_input = 0;
			}
		else {
			printf("\n[Client]Wrong input, try again...%c\n",msg_copy[0]);
		}
		
		if (write (sockfd, send_message, sizeof(send_message)) <= 0)
	    {
			perror ("\n[client]Eroare la write() spre server.\n");
			exit(0);    
		}
	} 
}

void checkStart(char msg[1024], int sockfd){
	int wrong_input = 1;
	while(wrong_input){//as long as the message does not contain the start word, it returns an error 
	    bzero (msg, 1024);
        printf ("\n[client]Write start when you are ready:");
        fflush (stdout);
        read (0, msg, 1024);
		char msg_copy[1025];
		bzero(msg_copy, 1024);
		snprintf(msg_copy, 1024, "%s", msg);
		char *position = strstr(msg_copy, "start");
		if(position == NULL){
			printf("[Client]Wrong input, try again...%s\n", msg_copy);
		}
		else if (write (sockfd, msg, sizeof(msg)) <= 0){
			perror ("\n[client]Eroare la write() spre server.\n");
		 	//return errno;
			exit(0);    
		}
		else{
			wrong_input = 0;
			printf("\n[client]The game will begin when all the clients are ready.\n");
		}
	} 
}

int checkWarnings(char msg[1024], int sockfd){
	int warnings = 0;
	while(1){	
		/*Reading the message from the server
		(blocking call until the server answers) */
		
		if (read (sockfd, msg, 1024) < 0)
		{
		 	perror ("[Client]Eroare la read() de la server.\n");
			exit(0);
		}
		msg[strlen(msg)] = '\0';
		if(strstr(msg, "You won!") != NULL){
			printf ("\n[client]You won! Cogratulations!\n");
			return 0;
		}
		
		warnings++;
		printf ("\n[client]Received warnings: %i\n", warnings);
		if(warnings == 3){
				printf ("\n[Client]You will be disconnected because you received 3 warnings.\n");
				return 0;
		}
	}
}

int main(int argc, char *argv[])
{
	int sockfd, connfd, avertizari;
	struct sockaddr_in servaddr, cli;
	char msg[1024];

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server..\n");

	//Reading the welcome message from the server:
	if (read (sockfd, msg, 1024) < 0){
		perror ("[client]Eroare la read() de la server.\n");
		exit(0);
	}
	if(strstr(msg, "The server is closed") != NULL){
		close(sockfd);
		exit(0);
	}
	printf ("\n[client]Received message: %s", msg);

	/*Reading first message: write 1 for Jom, 2 for Terry */
	checkNumber(msg, sockfd);

    //Citirea mesajului pentru start
	checkStart(msg, sockfd);

	bzero(msg, 1024);
	//If all the clients are  ready, receive a message:
	if (read (sockfd, msg, 1024) < 0){
		perror ("[client]Eroare la read() de la server.\n");
		//return errno;
		exit(0);
	}
	printf ("\n[Client]Received message: %s", msg);

    /* Warnings */
	//bzero (msg, 1024);
	checkWarnings(msg, sockfd);
    
	// close the socket
	close(sockfd);
}
