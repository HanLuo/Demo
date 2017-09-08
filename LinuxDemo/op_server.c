#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;

	char message[BUF_SIZE];

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket() error!");

	memset(&serv_sock, 0, sizeof(serv_sock));
	serv_sock.sin_family = AF_INET;
	serv_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_sock.sin_port = htons(atoi(argv[1]));

	if (bind(&serv_sock, (sockaddr*)&serv_sock, sizeof(serv_sock)) == -1)
		error_handling("bind() error!");

	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error!");

	while(1)
	{
		
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}