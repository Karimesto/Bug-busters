#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
int getColumnInput() {
    int col;

    while (1) {
        printf("Choose a column (1-7): ");

        if (scanf("%d", &col) != 1) {
            // Invalid input (letter, symbol, etc.)
            printf("Invalid input! Please enter a number from 1 to 7.\n");

            // Clear input buffer
            while (getchar() != '\n');

            continue;
        }

        // Valid number, check range
        if (col >= 1 && col <= 7)
            return col - 1;

        printf("Number must be between 1 and 7.\n");
    }
}


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
        int dr = direction[d][0];
        int dc = direction[d][1];
        int count = 1;

        int r = row + dr, c = col + dc;
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) {
            count++;
            r += dr;
            c += dc;
        }

        r = row - dr, c = col - dc;
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) {
            count++;
            r -= dr;
            c -= dc;
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


int botmove_medium(int arr[6][7], int botPlayer, int humanPlayer) {

    static bool firstMove = true;
    int col, row;

    // First move = try center
    if (firstMove && arr[0][3] == 0) {
        firstMove = false;
        return 3;
    }
    firstMove = false;

    // 1) Block opponent winning move
    for (col = 0; col < 7; col++) {
        if (arr[0][col] == 0) {
            row = changefunction(arr, col, humanPlayer);
            if (row != -1 && checkwin(arr, row, col)) {
                arr[row][col] = 0;
                return col;
            }
            if (row != -1) arr[row][col] = 0;
        }
    }

    // 2) Try to create 3-in-a-row for bot
    for (col = 0; col < 7; col++) {
        if (arr[0][col] == 0) {
            row = changefunction(arr, col, botPlayer);
            if (row == -1) continue;

            int direction[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
            int maxCount = 1;

            for (int d = 0; d < 4; d++) {
                int dr = direction[d][0];
                int dc = direction[d][1];
                int count = 1;

                int r = row + dr, c = col + dc;
                while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == botPlayer) {
                    count++; r += dr; c += dc;
                }

                r = row - dr; c = col - dc;
                while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == botPlayer) {
                    count++; r -= dr; c -= dc;
                }

                if (count > maxCount) maxCount = count;
            }

            arr[row][col] = 0;
            if (maxCount >= 3) return col;
        }
    }

    // 3) Priority columns (center first)
    int priority[7] = {3,2,4,1,5,0,6};
    for (int i = 0; i < 7; i++) {
        col = priority[i];
        if (arr[0][col] == 0)
            return col;
    }

    // 4) Fallback random
    do {
        col = rand() % 7;
    } while (arr[0][col] != 0);

    return col;
}


int evaluate_position(int arr[6][7], int player) {
    int score = 0;
    int centerCount = 0;

    for (int i = 0; i < 6; i++) {
        if (arr[i][3] == player) centerCount++;
    }
    score += centerCount * 4;

    int direction[4][2] = {{0,1},{1,0},{1,1},{1,-1}};

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 7; c++) {
            for (int d = 0; d < 4; d++) {
                int dr = direction[d][0];
                int dc = direction[d][1];
                int countPlayer = 0, countEmpty = 0;
                int valid = 1;

                for (int k = 0; k < 4; k++) {
                    int rr = r + dr*k;
                    int cc = c + dc*k;

                    if (rr < 0 || rr >= 6 || cc < 0 || cc >= 7) {
                        valid = 0;
                        break;
                    }

                    if (arr[rr][cc] == player) countPlayer++;
                    else if (arr[rr][cc] == 0) countEmpty++;
                }

                if (!valid) continue;

                if (countPlayer == 4) score += 10000;
                else if (countPlayer == 3 && countEmpty == 1) score += 50;
                else if (countPlayer == 2 && countEmpty == 2) score += 10;
                else if (countPlayer == 1 && countEmpty == 3) score += 1;
            }
        }
    }

    return score;
}

int evaluate_board(int arr[6][7], int botPlayer) {
    int opponent = (botPlayer == 1 ? 2 : 1);
    return evaluate_position(arr, botPlayer) - evaluate_position(arr, opponent);
}

int minimax(int arr[6][7], int depth, int alpha, int beta,
            int maximizingPlayer, int botPlayer,
            int lastRow, int lastCol) {

    int opponent = (botPlayer == 1 ? 2 : 1);

    if (lastRow != -1 && lastCol != -1 && arr[lastRow][lastCol] != 0) {
        if (checkwin(arr, lastRow, lastCol)) {
            if (arr[lastRow][lastCol] == botPlayer)
                return 1000000 + depth;
            else
                return -1000000 - depth;
        }
    }

    int full = 1;
    for (int c = 0; c < 7; c++) {
        if (arr[0][c] == 0) {
            full = 0;
            break;
        }
    }

    if (depth == 0 || full)
        return evaluate_board(arr, botPlayer);

    int priority[7] = {3,2,4,1,5,0,6};

    if (maximizingPlayer) {
        int bestScore = -100000000;

        for (int i = 0; i < 7; i++) {
            int col = priority[i];
            if (arr[0][col] != 0) continue;

            int row = changefunction(arr, col, botPlayer);
            if (row == -1) continue;

            int score = minimax(arr, depth - 1, alpha, beta,
                                0, botPlayer, row, col);

            arr[row][col] = 0;

            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break;
        }

        return bestScore;

    } else {

        int bestScore = 100000000;

        for (int i = 0; i < 7; i++) {
            int col = priority[i];
            if (arr[0][col] != 0) continue;

            int row = changefunction(arr, col, opponent);
            if (row == -1) continue;

            int score = minimax(arr, depth - 1, alpha, beta,
                                1, botPlayer, row, col);

            arr[row][col] = 0;

            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;
        }

        return bestScore;
    }
}


int botmove_hard(int arr[6][7]) {
    int botPlayer = 2;

    int bestScore = -100000000;
    int bestCol = 3;
    int alpha = -100000000;
    int beta  = 100000000;

    int priority[7] = {3,2,4,1,5,0,6};

    for (int i = 0; i < 7; i++) {
        int col = priority[i];
        if (arr[0][col] != 0) continue;

        int row = changefunction(arr, col, botPlayer);
        if (row == -1) continue;

        int score = minimax(arr, 7, alpha, beta, 0, botPlayer, row, col);

        arr[row][col] = 0;

        if (score > bestScore) {
            bestScore = score;
            bestCol = col;
        }
        if (score > alpha) alpha = score;
    }

    return bestCol;
}


int main() {
    int arr[6][7] = {0};
    int current = 1;
    int mode, difficulty = 1;

    srand(time(NULL));

    printf("Choose mode:\n1. Player vs Player\n2. Player vs Bot\n");
    scanf("%d", &mode);

    int humanPlayer = 1;
    int botPlayer = 2;

    if (mode == 2) {
        printf("Choose bot difficulty:\n1. Easy\n2. Medium\n3. Hard\n");
        scanf("%d", &difficulty);

        printf("Do you want to play first or second?\n");
        printf("1. Play first (You = A)\n");
        printf("2. Play second (You = B)\n");

        int choice;
        scanf("%d", &choice);

        if (choice == 2) {
            humanPlayer = 2;
            botPlayer = 1;
        }

        printf("\nYou are Player %c. Bot is Player %c.\n",
               (humanPlayer == 1 ? 'A' : 'B'),
               (botPlayer == 1 ? 'A' : 'B'));
    }

    while (true) {

        createboard(arr);

        int col;

        if (current == humanPlayer || mode == 1) {
            printf("Player %c, choose a column (1-7): ",
                   current == 1 ? 'A' : 'B');
            col = getColumnInput();

        }
        else {
            if (difficulty == 1) col = botmove_easy(arr);
            else if (difficulty == 2) col = botmove_medium(arr, botPlayer, humanPlayer);
            else col = botmove_hard(arr);

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

            if (mode == 2 && current == botPlayer)
                printf("Bot wins!\n");
            else if (mode == 2 && current == humanPlayer)
                printf("You win!\n");
            else
                printf("Player %c wins!\n", current == 1 ? 'A' : 'B');

            break;
        }

        current = (current == 1 ? 2 : 1);
    }

    return 0;
}
