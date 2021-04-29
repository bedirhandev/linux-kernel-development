#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    FILE* fd;
    int result;
    const char* filename;
    const char* message;
    char* buffer;

    filename = argv[1];
    message = argv[2];

    /* WRITE */
    printf("Opening %s\n", filename);
    fd = fopen(filename, "w");
    if(fd == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Writing %s\n", message);
    result = fwrite(message, 1, strlen(message), fd);
    if(result < 0)
    {
        printf("Error writing %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Succesfully writing %d bytes.\n", result);

    printf("Closing %s\n", filename);
    result = fclose(fd);
    if(result < 0)
    {
        printf("Error closing %s %s(%d)\n", filename, strerror(errno), errno);
    }
    
    printf("Successfully closed %s\n", filename);

    /* READ */
    printf("Opening %s\n", filename);
    fd = fopen(filename, "r");
    if(fd == NULL)
    {
        printf("Error opening %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Reading %s\n", message);
    result = fread(&buffer, sizeof(char), strlen(message), fd);
    
    if(result < 0)
    {
        printf("Error reading %s %s(%d)\n", filename, strerror(errno), errno);
    }

    printf("Succesfully reading %d bytes.\n", result);

    printf("Closing %s\n", filename);
    result = fclose(fd);
    if(result < 0)
    {
        printf("Error closing %s %s(%d)\n", filename, strerror(errno), errno);
    }
    
    printf("Successfully closed %s\n", filename);
}