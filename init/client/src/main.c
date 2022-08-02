#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


// rewrite a client in c++!!!!!!!!!!!!!
int main()
{
	for (int i = 0; i < 2000; ++i)
	{
		// system("nc localhost 8888 &");
		pid_t pid = fork();
		if (pid == 0)
		{
			while (true)
			{
				execlp("nc", "nc", "localhost", "8000", NULL);
				exit(-1);
			}
		}
		else if (pid == -1)
		{
			dprintf(2, "fork %d failed\n", i);
		}
		else
			printf("child %d\n", i);
	}
	wait(NULL);
	return (0);
}
