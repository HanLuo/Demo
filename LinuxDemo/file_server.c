#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int clnt_sock, serv_sock;
	struct sockaddr_in clnt_addr, serv_addr;

	char buf[BUF_SIZE];

	FILE* fp;
	socklen_t clnt_addr_sz;
	int read_cnt;

	fp = fopen("file_server.c", "rb");
	if (fp = nullptr)
		error_handling("open file error!");

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket() error!");

	memset(&serv_sock, 0, sizeof(serv_sock));
	serv_sock.sin_family = AF_INET;
	serv_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_sock.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, struct(sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error!");

	listen(serv_sock, 5);

	clnt_addr_sz = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);

	while(1)
	{
		read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
		if (read_cnt < BUF_SIZE)
		{
			write(clnt_sock, buf, read_cnt);
			break;
		}
		write(clnt_sock, buf, BUF_SIZE);
	}
	shutdown(clnt_sock, SHUT_WR);
	read(clnt_sock, buf, BUF_SIZE);
	printf("message from client: %s \n", buf);

	fclose(fp);
	close(clnt_sock);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}