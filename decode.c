#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BLOCK_SIZE 8

int map(char encChar);
char shiftChar(char ascii);

int main(int argc, const char **argv)
{
  if (argc != 2)
  {
    printf("Usage: decode FILE");
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);

  if (fd == -1)
  {
    switch (errno)
    {
      case EACCES:
        printf("Error: Access Denied\n");
        break;
      case EINTR:
        printf("Error: Interrupted\n");
        break;
    }
    return 2;
  }

  char buffer[BLOCK_SIZE];
  int shift = 0;
  int check = 0;
  int sum = 0;
  while (1)
  {
    ssize_t readbytes = read(fd, buffer, BLOCK_SIZE);
   
    if (readbytes == -1)
    {
      switch (errno)
      {
        case EBADF:
          printf("Bad file descriptor\n");
          break;
        default:
          printf("Errno: %u", errno);
      }
    }

    for (int i = 0; i < readbytes; ++i)
    {
      int mapping = map(buffer[i]);

      if (check)
      {

        if ((mapping - 48) == (sum % 10))
          printf("<good>");
        else
          printf("<bad:%d:%d>", mapping - 48, sum);
        sum = 0;
        check = 0;
        continue;
      }

      if (mapping == -3)
      {
        check = 1;
        continue;
      }

      sum += buffer[i];

      if (mapping == -1)
      {
        printf("<!>");
        continue;
      }
      else if (mapping == -2)
      {
        shift = !shift;
        continue;
      }
      else if (mapping == -4) // This character isn't even necessary for parsing.
      {
        continue;
      }
      
      char printChar = shift ? shiftChar(mapping) : mapping;
      printf("%c", printChar);
    }

    if (readbytes < BLOCK_SIZE)
      break;
  }

  close(fd);
  printf("\n");
  return 0;
}

/**
 * Maps an encoded character to an ASCII one.
 * Returns -1 for invalid characters.
 * Returns -2 for SHIFT.
 * Returns -3 for CBGN.
 * Returns -4 for CEND.
 */
int map(char encChar)
{
  /* Shortcut for alphanumerics by mapping onto ASCII */
  if (encChar > 0 && encChar < 27)
    return ((encChar - 1) % 26) + 97;
  else if (encChar > 29 && encChar < 40)
    return (encChar % 10) + 48;
  
  /* We can index this contiguous block of characters to save some time and space */
  if (encChar > 39 && encChar < 63)
  {
    char block[] = {
      '`', '~', '!', '@', '#', '$', '%', '^', '&', '*', '(',
      ')', '-', '_', '=', '+', '[', ']', '{', '}', '\\', '/', '|'
    };

    return block[encChar - 40];
  }

  switch (encChar)
  {
    case 0:
      return 0;
    case 27:
      return ' ';
    case 28:
      return -3;
    case 29:
      return -4;
    case 63:
      return -2;
  }

  return -1;
}

char shiftChar(char ascii)
{
  if (ascii > 96 && ascii < 123)
    return ascii - 32;
  else if (ascii > 64 && ascii < 91)
    return ascii + 32;
  else
    return ascii;
}
