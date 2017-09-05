#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int clnt_sock;

	struct sockaddr_in serv_addr;

	char message[30];

	int str_len = 0;
	int idex = 0, read_len = 0;
	int count = 0;

	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (clnt_sock == -1)
		error_handling("socket() error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	while(read_len = read(clnt_sock, &message[idex++], 1))
	{
		str_len += read_len;
		count += 1;
	}
	printf("Message from Servet: %s, Message len: %d\n", message, str_len);
	printf("Function read call count: %d\n", count);
	close(clnt_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}