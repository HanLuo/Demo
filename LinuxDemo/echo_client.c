#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int clnt_sock;
	struct sockaddr_in serv_addr;

	char mestoserver[BUF_SIZE];
	char gettoserver[BUF_SIZE];

	int message_size;

	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (clnt_sock == -1)
		error_handling("socket() error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(clnt_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");
	else
		puts("connect ...");

	while(1)
	{
		fputs("please input send message to server", stdout);
		fgets(mestoserver, BUF_SIZE, stdin);

		if (!strcmp(mestoserver, "q\n") || !strcmp(mestoserver, "Q\n"))
			break;

		write(clnt_sock, mestoserver, strlen(mestoserver));
		message_size = read(clnt_sock, gettoserver, BUF_SIZE-1);
		gettoserver[message_size] = 0;
		printf("message from server:%s", gettoserver);
	}
	close(clnt_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}