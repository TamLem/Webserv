#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	//get current working directory
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	//get environment variables
	char *file_name = getenv("PATH_INFO");
	if (file_name == NULL)
	{
		printf("No file name\n");
	}
	//read the file
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		printf("File not found\n");
	}
	char line[1024];
	while (fgets(line, sizeof(line), fp)) {
		printf("%s", line);
	}
	return 0;
}