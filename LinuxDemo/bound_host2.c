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
	int sock;
	struct sockaddr_in clnt_adr;
	char msg1[] = "111111";
	char msg2[] = "222222";
	char msg3[] = "333333";

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
		error_handling("socket() error!");

	memset(&clnt_adr, 0, sizeof(clnt_adr));
	clnt_adr.sin_family = AF_INET;
	clnt_adr.sin_addr.s_addr = inet_addr(argv[1]);
	clnt_adr.sin_port = htons(atoi(argv[2]));

	sendto(sock, msg1, sizeof(msg1), 0, (struct sockaddr*)&clnt_adr, sizeof(clnt_adr));
	sendto(sock, msg1, sizeof(msg2), 0, (struct sockaddr*)&clnt_adr, sizeof(clnt_adr));
	sendto(sock, msg1, sizeof(msg3), 0, (struct sockaddr*)&clnt_adr, sizeof(clnt_adr));
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}