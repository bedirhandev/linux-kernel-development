#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>

#define MESSAGE_SEQ_NO 0x01
#define MEM_MESSAGE _IO('M',MESSAGE_SEQ_NO)

int main(int argc, const char* argv[])
{
    int fd;
    int result;
    const char* filename;
	
    filename = argv[1];

    /* WRITE */
    printf("Opening %s with read/write mode\n", filename);
    fd = open(filename, O_RDWR);
	
    if(fd < 0)
    {
        printf("Error opening %s.\n", filename);
        return -1;
    }

    ioctl(fd, MEM_MESSAGE);

    printf("Closing %s\n", filename);
    close(fd);
    
    printf("Successfully closed %s\n", filename);

    return 0;
}