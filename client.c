// standard IO libraries
#include<string.h>
#include<stdlib.h>
// networking useful libraries
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<fcntl.h>
// Linux system library
#include<unistd.h>

#include "stack.h"


#define OUTRANGE(x) ((x<1) || (x>65535))

// Error handling auxiliar function
void mayday(char *s){
  perror(s);
  exit(1);
}


int getUser(char* buf){
  FILE *fp;
  /* Open the command for reading. */
  fp = popen("whoami", "r");
  if (fp == NULL) {
    printf("Failed to get user. Maybe you can't run whoami command. Check credentials\n" );
    return 0;
  }
  fscanf(fp, "%s" , buf); // maybe it's a bad idea to use scanf,
                          // buffer overflow can happen here.
  /* close */
  pclose(fp);
  return 1;
}

int main(int argc , char* argv[]){
  struct sockaddr_in pin_addr;
  struct sockaddr_in pout_addr;
  unsigned long local_port, remote_port;
  int tx, rx;
  char rx_buf[BUFLEN];
  char tx_buf[BUFLEN];
  char username[50];
  char c[1];
  Stack* st = newstack();
  // Checa quantidade de argumentos
  if (argc < 4){
    fprintf(stderr, "use como : ./topchat <port_envio> <host_remoto> <port_recv>\n");
    return 1;
  }
  local_port = strtoul(argv[3],NULL,0);
  remote_port = strtoul(argv[1],NULL,0);
  // Checa se portas estão dentro de intervalo
  if (OUTRANGE(local_port) || OUTRANGE(remote_port)){
    fprintf(stderr, "Numero de porta invalido\n");
  }
  // Tenta abrir socket
  if ((tx=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    mayday("Nao pode abrir socket()");
  }
  if ((rx=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    mayday("Nao pode abrir socket()");
  }
  // Set socket to non-blocking 
  fcntl(rx, F_SETFL, O_NONBLOCK);
  fcntl(tx, F_SETFL, O_NONBLOCK);
  // Set stdin to non-blocking too
  fcntl(0, F_SETFL, O_NONBLOCK);
  // tx socket parameters setting
  memset(&pout_addr, 0, sizeof(pout_addr));
  pout_addr.sin_family = AF_INET;
  pout_addr.sin_port = htons(remote_port);
  // you got to set his address, otherwise who is him?
  if (inet_aton(argv[2], &pout_addr.sin_addr) == 0) {
    printf("Endereço invalido %s\n", argv[2]);
    return 1;
  }
  // rx socket parameters setting
  memset(&pin_addr, 0, sizeof(pin_addr));
  pin_addr.sin_family = AF_INET;
  pin_addr.sin_port = htons(local_port);
  // you know who you are, so pick any address (OS takes care of it)
  pin_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(rx, (struct sockaddr*)(&pin_addr), sizeof pin_addr);
  // clear buffer so first input is free of litter
  memset(rx_buf, 0 , sizeof rx_buf);
  if(!getUser(username)) strcpy(username, "Anonymous");
  char aux[256];
  while(1){
    // Buffer for stdin reading
    // condition is if stdin is clear
    while(read(0,c,1)>0){
      if (c[0]=='\n'){
        // if endline , flushes stack into tx_buf for printing
        flush(st,tx_buf);
        printf ("[%s]:%s\n",username,tx_buf);
        sprintf(aux, "[%s]:%s\n",username,tx_buf);
        if (sendto(tx, aux , strlen(aux) , 0 , (struct sockaddr *) &pout_addr, sizeof(pout_addr))==-1) {
          mayday("sendto()");
        }
      } else {
        // if not endline, puts into stack buffer
        push(st,c[0]);
      }
    }

    // Nonblocking reading for receiving messages
    if (read(rx,rx_buf,sizeof(rx_buf)) > 0){
      printf ("%s\n",rx_buf);
      memset(rx_buf, 0 , sizeof rx_buf);
    }
  }

  return 0;
}
