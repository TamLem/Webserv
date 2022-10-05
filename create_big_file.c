#include "unistd.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char **argv)
{
    (void)argc;
    //create a file size of argv[1] bytes
    int fd = open("big_file", O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        printf("open file failed");
        exit(1);
    }
    int size = atoi(argv[1]);
    for (int i=0; i < size; i++)
    {
        write(fd, "a", 1);
    }
    close(fd);
    return 0;
}