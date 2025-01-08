#include <sys/socket.h>  // Core socket functions (socket, bind, listen, accept)
#include <netinet/in.h>  // Structures for address families (sockaddr_in, htons)
#include <arpa/inet.h>   // IP address conversion (inet_pton, inet_ntoa)
#include <unistd.h>      // For close() to close sockets
#include <stdio.h>       // For printf(), perror(), etc.
#include <stdlib.h>      // For exit() and memory allocation
#include <string.h>      // For memset() and string operations
#include <fcntl.h>
#include <unistd.h>
void receive_file();
void send_file(char* dest_ip, char* file_name);
//int setsockopt(int sockfd, int level, int optname,  const void *optval, socklen_t optlen);

int main(int argc, char* argv[]){
    if(argc==0){
        receive_file();
    }
    else if(argc==2){
        send_file(argv[1], argv[2]);
    }
    else{
        printf("Invalid number of arguments");
        return 1;
    }
    return 0;
    //first argument is file, second is dest. ip address

}
void receive_file(){
    int fd_socket;
    struct sockaddr_in addr={.sin_family=AF_INET,.sin_addr.s_addr=INADDR_ANY,.sin_port=htons(42069)};
    int addr_length=sizeof(addr);
    if ((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (bind(fd_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(fd_socket);
        exit(EXIT_FAILURE);
    }
    if (listen(fd_socket, 3) < 0) {
        perror("Listen failed");
        close(fd_socket);
        exit(EXIT_FAILURE);
    }
    int client_socket=accept(fd_socket, (struct sockaddr *)&addr, (socklen_t *)&addr_length);
    char file_name[64];

    read(client_socket, file_name, 64);
    int file_begin=-1;
    for(int i=0; i<64; i++) {
        if(file_name[i]=='\0'){
            file_begin=i+1;
            break;
        }
    }
    int file_created=open(file_name,O_CREAT,0666);
    //move socket fd to point to beginning of file
    lseek(client_socket, file_begin, SEEK_SET);
    char buff[1024];
    int bytes_read,bytes_written;
    while((bytes_read=read(client_socket, buff, 1024))>0){
        bytes_written=write(file_created, buff, bytes_read);
    }
    close(client_socket);
    close(file_created);
    printf("file %s received successfully",file_name);
}
void send_file(char* dest_ip, char* file_name){
    struct sockaddr_in server_address;
    int sockfd =socket(AF_INET,SOCK_STREAM,0);
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(42069);
    if (inet_pton(AF_INET, dest_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }
    if(connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address))<0){
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    //first send filename terminated by null character
    send(sockfd,file_name,strlen(file_name)+1,0);
    int file=open(file_name,O_RDONLY);
    off_t bytesToBeRead=0;//0 means to read until end of file
    sendfile(sockfd, file, 0 , &bytesToBeRead, 0, 0);
    printf("message sent\n");
    close(sockfd);
    close(file);


}
