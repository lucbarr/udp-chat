// standard IO libraries
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<malloc.h>
// networking useful libraries
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<fcntl.h>
// Linux system library
#include<unistd.h>

#define SERVER "172.16.36.46"
#define BUFLEN 512  //Max length of buffer
#define PORT 31337   //The port on which to send data

#define OUTRANGE(x) ((x<1) || (x>65535))

void mayday(char *s){
  perror(s);
  exit(1);
}

int inputAvailable() {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}
// Definition of stack for i/o nonblocking reading
typedef struct Stack{
  char buf[BUFLEN];
  size_t top;
} Stack;

Stack* newstack(){
  Stack* new_stack = (Stack*) malloc(sizeof(Stack));
  memset(new_stack,0,sizeof(*new_stack));
  return new_stack;
}

void closestack(Stack* stack){
  free(stack);
}

void push(Stack* stack, char c){
  if (stack->top >= BUFLEN)
    fprintf(stderr, "push() error: buffer overflow" );
  else{
    stack->buf[stack->top]=c;
    stack->top ++;
  }
}

void flush(Stack* stack, char* buf){
  strcpy(buf,stack->buf);
  memset(stack,0,sizeof(*stack));
}
////////////////////////////////////////////////////

int main(int argc , char* argv[]){
  struct sockaddr_in pin_addr;
  struct sockaddr_in pout_addr;
  unsigned long local_port, remote_port;
  int tx, rx;
  char rx_buf[BUFLEN];
  char tx_buf[BUFLEN];
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
  // Set socket to non-blocking //
  fcntl(rx, F_SETFL, O_NONBLOCK);
  fcntl(tx, F_SETFL, O_NONBLOCK);
  fcntl(0, F_SETFL, O_NONBLOCK);
  // Inicializa sockaddr_in de saida (host remoto)
  memset(&pout_addr, 0, sizeof(pout_addr));
  pout_addr.sin_family = AF_INET;
  pout_addr.sin_port = htons(remote_port);
  if (inet_aton(argv[2], &pout_addr.sin_addr) == 0) {
    printf("Endereço invalido %s\n", argv[2]);
    return 1;
  }
  // Inicializa sockaddr_in de entrada
  memset(&pin_addr, 0, sizeof(pin_addr));
  pin_addr.sin_family = AF_INET;
  pin_addr.sin_port = htons(local_port);
  pin_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(rx, (struct sockaddr*)(&pin_addr), sizeof pin_addr);
  memset(rx_buf, 0 , sizeof rx_buf);
  char c[1];
  while(1){
    while(read(0,c,1)>0){
      if (c[0]=='\n'){
        flush(st,tx_buf);
        printf ("Message:%s\n",tx_buf);
        if (sendto(tx, tx_buf , strlen(tx_buf) , 0 , (struct sockaddr *) &pout_addr, sizeof(pout_addr))==-1) {
          mayday("sendto()");
        }
      } else {
        push(st,c[0]);
      }
    }
    if (read(rx,rx_buf,sizeof(rx_buf)) > 0){
      printf ("Received message: %s\n",rx_buf);
      memset(rx_buf, 0 , sizeof rx_buf);
    }
  }

  return 0;
}
