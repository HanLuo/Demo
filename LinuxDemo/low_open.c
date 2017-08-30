#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void error_handling(char *message);

int main(void)
{
	int fd;
	char buf[] = "test open file!";

	fd = open("1.txt", O_CREAT|O_WRONLY|O_TRUNC);
	if (fd == -1)
		error_handling("open file error!");

	printf("file descripter: %d\n", fd);

	if (write(fd, buf, sizeof(buf)) == -1)
		error_handling("write file error!");

	close(fd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}