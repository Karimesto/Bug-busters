// ===== client.c =====

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);

    printf("Enter server IP: ");
    char ip[64];
    scanf("%s",ip);

    inet_aton(ip,&addr.sin_addr);

    printf("Connecting...\n");
    connect(sock,(struct sockaddr*)&addr,sizeof(addr));
    printf("Connected to server.\n");

    int send_col, recv_col;

    while(1)
    {
        // YOU play first
        printf("Your column: ");
        scanf("%d",&send_col);

        send(sock,&send_col,sizeof(int),0);

        // wait for opponent
        recv(sock,&recv_col,sizeof(int),0);
        printf("Opponent played column %d\n", recv_col);
    }

    close(sock);
    return 0;
}
