

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {

    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server,1);

    printf("Waiting for client...\n");
    int client = accept(server,NULL,NULL);
    printf("Client connected.\n");

    int send_col, recv_col;

    while(1) {

        // wait for opponent move
        recv(client, &recv_col, sizeof(int), 0);
        printf("Opponent played column %d\n", recv_col);

        // then ask user to play
        printf("Your column: ");
        scanf("%d",&send_col);

        send(client,&send_col,sizeof(int),0);
    }

    close(client);
    close(server);
    return 0;
}
