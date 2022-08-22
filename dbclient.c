#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "msg.h"
#define BUF 256

void Usage(char *progname);

int LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen);

int Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd);

void put();
void get();
void delete();
int socket_fd;
int 
main(int argc, char **argv) {
  if (argc != 3) {
    Usage(argv[0]);
  }

  unsigned short port = 0;
  if (sscanf(argv[2], "%hu", &port) != 1) {
    Usage(argv[0]);
  }

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  size_t addrlen;
  if (!LookupName(argv[1], port, &addr, &addrlen)) {
    Usage(argv[0]);
  }

  // Connect to the remote host.
  if (!Connect(&addr, addrlen, &socket_fd)) {
    Usage(argv[0]);
  }

  // Read something from the remote host.
  // Will only read BUF-1 characters at most.
  int32_t index;
  while (1)//Prompt user for input to perform put, get, or delete on database
  {
    printf("\nEnter your choice (1 to put, 2 to get, 3 to delete, 0 to quit): ");
    scanf("%d",&index);
    getchar();
    if (index == 1)
    {
       //Send put data to server
       put();
    }
    else if (index == 2)
    {
       //Send get data to server
       get();
    }
    else if (index == 3)
    {
       //Delete record from database
       delete();                  
    }
    else
    {
       break;
    }
  }
  // Clean up.
  close(socket_fd);
  return EXIT_SUCCESS;
}
void put()//Send msg struct to server indicating data to be written to database. Success/failure message is sent back from server.
{
    struct msg m;
    printf("Enter the name: ");
    fgets(m.rd.name, MAX_NAME_LENGTH, stdin);
    m.rd.name[strlen(m.rd.name)-1] = '\0';
    printf("\nEnter the id: ");
    scanf("%d",&m.rd.id);
    m.type = 1;
    int wres = write(socket_fd, &m, sizeof(m));
    if (wres == 0) {
     printf("socket closed prematurely \n");
     close(socket_fd);
     return;
    }
    if (wres == -1) {
      if (errno == EINTR)
        return;
      printf("socket write failure \n");
      close(socket_fd);
      return;
    }
    int res = read(socket_fd, &m, sizeof(m));
     if (res == 0) {
       printf("socket closed prematurely \n");
       close(socket_fd);
       return;
     }
     if (res == -1) {
       if (errno == EINTR)
         return;
       printf("socket read failure \n");
       close(socket_fd);
       return;
     }
     if (m.type == 4)
     {
       printf("\nPut success.");
     }
     else if (m.type == 5)
     {
       printf("\nPut failed.");
     }
}

void get()//Send msg struct to server indicating data to be displayed from database. Success/failure message is sent back from server.
{
   struct msg m;
   printf("Enter the id: ");
   scanf("%d", &m.rd.id);
   getchar();
  m.type = 2;
   int wres = write(socket_fd, &m, sizeof(m));
   if (wres == 0) {
     printf("socket closed prematurely \n");
     close(socket_fd);
     return;
   }
   if (wres == -1) {
     if (errno == EINTR)
       return;
     printf("socket write failure \n");
     close(socket_fd);
     return;
   }
   int res = read(socket_fd, &m, sizeof(m));
    if (res == 0) {
     printf("socket closed prematurely \n");
     close(socket_fd);
     return;
    }
    if (res == -1) {
      if (errno == EINTR)
        return;
      printf("socket read failure \n");
      close(socket_fd);
      return;
    }
     if (m.type == 4)
     {
        printf("\nname: %s", m.rd.name);
	printf("\nid: %d", m.rd.id);
     }
     else if (m.type == 5)
     {
        printf("\nGet failed.");
     }
}

void delete()////Send msg struct to server indicating data to be removed database. Success/failure message is sent back from server.
{
  //implement "unused" overwrite
  struct msg m;
  printf("Enter the id: ");
  scanf("%d", &m.rd.id);     
  getchar();
 m.type = 3;
  int wres = write(socket_fd, &m, sizeof(m));
  if (wres == 0) {
    printf("socket closed prematurely \n");
    close(socket_fd);
    return;
  }
  if (wres == -1) {
    if (errno == EINTR)
      return;
    printf("socket write failure \n");
    close(socket_fd);
    return;
  }
  int res = read(socket_fd, &m, sizeof(m));
   if (res == 0) {
     printf("socket closed prematurely \n");
     close(socket_fd);
     return;
   }
   if (res == -1) {
     if (errno == EINTR)
       return;
     printf("socket read failure \n");
     close(socket_fd);
     return;
   }
   if (m.type == 4)
   {
     printf("\nname: %s", m.rd.name);
     printf("\nid: %d", m.rd.id);
   }
   else if (m.type == 5)
   {
     printf("\nDelete failed.");
   }
}
void 
Usage(char *progname) {
  printf("usage: %s  hostname port \n", progname);
  exit(EXIT_FAILURE);
}

int 
LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen) {
  struct addrinfo hints, *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Do the lookup by invoking getaddrinfo().
  if ((retval = getaddrinfo(name, NULL, &hints, &results)) != 0) {
    printf( "getaddrinfo failed: %s", gai_strerror(retval));
    return 0;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr =
            (struct sockaddr_in *) (results->ai_addr);
    v4addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr =
            (struct sockaddr_in6 *)(results->ai_addr);
    v6addr->sin6_port = htons(port);
  } else {
    printf("getaddrinfo failed to provide an IPv4 or IPv6 address \n");
    freeaddrinfo(results);
    return 0;
  }

  // Return the first result.
  assert(results != NULL);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return 1;
}

int 
Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd) {
  // Create the socket.
  int socket_fd = socket(addr->ss_family, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("socket() failed: %s", strerror(errno));
    return 0;
  }

  // Connect the socket to the remote host.
  int res = connect(socket_fd,
                    (const struct sockaddr *)(addr),
                    addrlen);
  if (res == -1) {
    printf("connect() failed: %s", strerror(errno));
    return 0;
  }

  *ret_fd = socket_fd;
  return 1;
}
