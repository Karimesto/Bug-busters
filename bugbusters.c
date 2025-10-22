#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>


void createboard(int arr[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (arr[i][j] == 0) printf(". ");
            else if (arr[i][j] == 1) printf("A ");
            else if (arr[i][j] == 2) printf("B ");
        }
        printf("\n");
    }
    printf("1 2 3 4 5 6 7\n");
}


int changefunction(int arr[6][7], int col, int player) {
    for (int row = 5; row >= 0; row--) {
        if (arr[row][col] == 0) {
            arr[row][col] = player;
            return row;
        }
    }
    return -1;
}


bool checkwin(int arr[6][7], int row, int col) {
    int player = arr[row][col];
    int direction[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for (int d = 0; d < 4; d++) {
        int drow = direction[d][0];
        int dcol = direction[d][1];
        int count = 1;


        int r = row + drow;
        int c = col + dcol;
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) {
            count++; r += drow; c += dcol;
        }


        r = row - drow; c = col - dcol;
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) {
            count++; r -= drow; c -= dcol;
        }


        if (count >= 4) return true;
    }
    return false;
}


int botmove_easy(int arr[6][7]) {
    int col;
    do {
        col = rand() % 7;
    } while (arr[0][col] != 0);  
    return col;
}


int main() {
    int arr[6][7] = {0};
    int current = 1;
    int mode, difficulty = 1;


    srand(time(NULL));


    printf("Choose mode:\n1. Player vs Player\n2. Player vs Bot\n");
    scanf("%d", &mode);


    if (mode == 2) {
        printf("Choose bot difficulty:\n1. Easy\n(Other levels coming soon!)\n");
        scanf("%d", &difficulty);
        printf("\nYou are Player A. The Bot is Player B.\n\n");
    }


    while (true) {
        createboard(arr);


        int col;
        if (current == 1 || mode == 1) {
            printf("Player %c, choose a column (1-7): ", current == 1 ? 'A' : 'B');
            scanf("%d", &col);
            col -= 1;
        } else {
            col = botmove_easy(arr);
            printf("Bot chooses column %d\n", col + 1);
        }


        if (col < 0 || col >= 7) {
            printf("Invalid column! Try again.\n");
            continue;
        }


        int row = changefunction(arr, col, current);
        if (row == -1) {
            printf("Column full! Try again.\n");
            continue;
        }


        if (checkwin(arr, row, col)) {
            createboard(arr);
            if (mode == 2 && current == 2)
                printf("Bot wins!\n");
            else
                printf("Player %c wins!\n", current == 1 ? 'A' : 'B');
            break;
        }


        current = (current == 1) ? 2 : 1;
    }


    return 0;
}





