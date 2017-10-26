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
	int serv_sock;
	socklen_t clnt_adr_sz;
	int str_len;

	struct sockaddr_in serv_addr, clnt_addr;
	char message[BUF_SIZE];

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (serv_sock == -1)
		error_handling("socket() error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(argv[1]);

	if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error!");

	while(1)
	{
		clnt_adr_sz = sizeof(clnt_addr);
		str_len = recvfrom(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_adr_sz);
		sendto(serv_sock, (struct sockaddr*)&clnt_addr, clnt_adr_sz);
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