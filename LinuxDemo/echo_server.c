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
	int serv_sock, clnt_sock,
	struct sockaddr_in serv_addr, clnt_addr;
	int clnt_sock_size;

	char message[BUF_SIZE];
	int str_len;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error!");

	if (listen(serv_addr, 10) == -1)
		error_handling("listen() error!");

	for (int i = 0; i < 5; i++)
	{
		clnt_sock_size = sizeof(clnt_addr);
		if ((clnt_sock = accept(serv_sock, (sockaddr*)&clnt_addr, &clnt_sock_size)) == -1)
			error_handling("accept() error!");

		printf("++++++++++++++++clnt_sock++++++++++++++++: %d\n", clnt_sock);

		while((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
		{
			write(clnt_sock, message, sizeof(message));
		}
		close(clnt_sock);
	}

	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}