#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, const char* argv[])
{
    FILE* fd;
	FILE* fd1;

    int result;
    const char* filename;
    const char* message;
    char *buffer;
	char *buffer1;

    filename = argv[1];
    message = argv[2];
	const char* message2 = "doei";

    /* WRITE */
    printf("Opening %s with read/write mode\n", filename);
    fd = fopen(filename, "rw+");
	fd1 = fopen(filename, "rw+");
    if(fd == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

	if(fd1 == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Writing %s\n", message);
    result = fwrite(message, 1, strlen(message), fd);
	result = fwrite(message2, 1, strlen(message2), fd1);
    if(result < 0)
    {
        printf("Error writing %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Succesfully writing %d bytes.\n", result);

    printf("Closing %s\n", filename);

    //result = fclose(fd);

    if(result < 0)
    {
        printf("Error closing %s %s(%d)\n", filename, strerror(errno), errno);
    }
    
    printf("Successfully closed %s\n", filename);

    /* READ */
    buffer = malloc(sizeof(char) * strlen(message));
	buffer1 = malloc(sizeof(char) * strlen(message2));

    //printf("Opening %s\n", filename);
    //fd = fopen(filename, "r");
    if(fd == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

	if(fd1 == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Reading %s\n", message);
    result = fread(buffer, 1, strlen(message), fd);

    fseek(fd1, 1, SEEK_SET);

	result = fread(buffer1, 1, strlen(message2), fd1);
	
	printf("Buffer: %s", buffer);
    printf("Buffer1: %s", buffer1);
    
    if(result < 0)
    {
        printf("Error reading %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Succesfully reading %d bytes.\n", result);

    printf("Closing %s\n", filename);
    result = fclose(fd);
	result = fclose(fd1);
    free(buffer);
	free(buffer1);
    if(result < 0)
    {
        printf("Error closing %s %s(%d)\n", filename, strerror(errno), errno);
    }
    
    printf("Successfully closed %s\n", filename);
}