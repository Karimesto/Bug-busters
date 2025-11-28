#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define TABLE_SIZE 100003
#define MAX_DEPTH 5


typedef struct {
    char key[43];
    int value;
    int depth;
} TTEntry;
static TTEntry transpositionTable[TABLE_SIZE];
static pthread_rwlock_t tt_lock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct {
    int arr[6][7];
    int col;
    int botPlayer;
    int score;
    int depth;       // search depth for this task
} ThreadTask;

int hashKey(const char *key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;
    return (int)(hash % TABLE_SIZE);
}

void boardToString(int arr[6][7], char *key) {
    int k = 0;
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 7; j++)
            key[k++] = '0' + arr[i][j];
    key[42] = '\0';
}

int getColumnInput() {
    int col;
    while (1) {
        printf("Choose a column (1-7): ");
        if (scanf("%d", &col) != 1) {
            printf("Invalid input! Please enter a number from 1 to 7.\n");
            while (getchar() != '\n');
            continue;
        }
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
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) { count++; r+=dr; c+=dc; }
        r = row - dr; c = col - dc;
        while (r >= 0 && r < 6 && c >= 0 && c < 7 && arr[r][c] == player) { count++; r-=dr; c-=dc; }
        if (count >= 4) return true;
    }
    return false;
}

int botmove_easy(int arr[6][7]) {
    int col;
    do { col = rand() % 7; } while (arr[0][col] != 0);
    return col;
}

int botmove_medium(int arr[6][7], int botPlayer, int humanPlayer) {
    static bool firstMove = true;
    int col, row;
    if (firstMove && arr[0][3] == 0) { firstMove = false; return 3; }
    firstMove = false;

    for (col = 0; col < 7; col++) {
        if (arr[0][col] == 0) {
            row = changefunction(arr, col, humanPlayer);
            if (row != -1 && checkwin(arr, row, col)) { arr[row][col] = 0; return col; }
            if (row != -1) arr[row][col] = 0;
        }
    }

    for (col = 0; col < 7; col++) {
        if (arr[0][col] == 0) {
            row = changefunction(arr, col, botPlayer);
            if (row == -1) continue;
            int direction[4][2] = {{0,1},{1,0},{1,1},{1,-1}};
            int maxCount = 1;
            for (int d = 0; d < 4; d++) {
                int dr = direction[d][0], dc = direction[d][1], count = 1;
                int r = row + dr, c = col + dc;
                while (r>=0 && r<6 && c>=0 && c<7 && arr[r][c]==botPlayer){ count++; r+=dr; c+=dc; }
                r = row - dr; c = col - dc;
                while (r>=0 && r<6 && c>=0 && c<7 && arr[r][c]==botPlayer){ count++; r-=dr; c-=dc; }
                if (count>maxCount) maxCount=count;
            }
            arr[row][col] = 0;
            if (maxCount>=3) return col;
        }
    }

    int priority[7] = {3,2,4,1,5,0,6};
    for (int i=0;i<7;i++){ col=priority[i]; if(arr[0][col]==0) return col; }

    do { col = rand() % 7; } while (arr[0][col] != 0);
    return col;
}

int evaluate_position(int arr[6][7], int player) {
    int score=0, centerCount=0;
    for(int i=0;i<6;i++) if(arr[i][3]==player) centerCount++;
    score+=centerCount*4;
    int direction[4][2] = {{0,1},{1,0},{1,1},{1,-1}};
    for(int r=0;r<6;r++) for(int c=0;c<7;c++) for(int d=0;d<4;d++){
        int dr=direction[d][0], dc=direction[d][1];
        int countPlayer=0, countEmpty=0, valid=1;
        for(int k=0;k<4;k++){
            int rr=r+dr*k, cc=c+dc*k;
            if(rr<0||rr>=6||cc<0||cc>=7){valid=0; break;}
            if(arr[rr][cc]==player) countPlayer++;
            else if(arr[rr][cc]==0) countEmpty++;
        }
        if(!valid) continue;
        if(countPlayer==4) score+=10000;
        else if(countPlayer==3 && countEmpty==1) score+=50;
        else if(countPlayer==2 && countEmpty==2) score+=10;
        else if(countPlayer==1 && countEmpty==3) score+=1;
    }
    return score;
}

int evaluate_board(int arr[6][7], int botPlayer) {
    int opponent=(botPlayer==1?2:1);
    return evaluate_position(arr,botPlayer)-evaluate_position(arr,opponent);
}

/* Forward declaration */
int minimax(int arr[6][7], int depth, int alpha, int beta,
            int maximizingPlayer, int botPlayer,
            int lastRow, int lastCol);

/* Thread worker: receives pointer to ThreadTask */
void* thread_worker(void *arg) {
    ThreadTask *task = (ThreadTask*)arg;

    // apply root move on this thread's board
    int row = changefunction(task->arr, task->col, task->botPlayer);
    if (row == -1) {
        task->score = -100000000; // invalid move (shouldn't happen)
        return NULL;
    }

    // After placing the move, it's opponent's turn -> maximizingPlayer = 0
    task->score = minimax(task->arr, task->depth, -100000000, 100000000,
                          0, task->botPlayer, row, task->col);

    // no need to undo since this is a private copy
    return NULL;
}

/* Thread-safe transposition table helpers (reader/writer lock around accesses) */
bool tt_lookup(const char *boardKey, int depth, int *out_value) {
    int idx = hashKey(boardKey);
    bool found = false;
    pthread_rwlock_rdlock(&tt_lock);
    if (strcmp(transpositionTable[idx].key, boardKey) == 0 && transpositionTable[idx].depth >= depth) {
        *out_value = transpositionTable[idx].value;
        found = true;
    }
    pthread_rwlock_unlock(&tt_lock);
    return found;
}

void tt_store(const char *boardKey, int depth, int value) {
    int idx = hashKey(boardKey);
    pthread_rwlock_wrlock(&tt_lock);
    strcpy(transpositionTable[idx].key, boardKey);
    transpositionTable[idx].value = value;
    transpositionTable[idx].depth = depth;
    pthread_rwlock_unlock(&tt_lock);
}

/* Minimax with alpha-beta and TT (locks only during TT lookup/store) */
int minimax(int arr[6][7], int depth, int alpha, int beta,
            int maximizingPlayer, int botPlayer,
            int lastRow, int lastCol) {

    char boardKey[43];
    boardToString(arr, boardKey);

    int cached;
    if (tt_lookup(boardKey, depth, &cached)) return cached;

    int opponent = (botPlayer == 1 ? 2 : 1);

    if (lastRow != -1 && lastCol != -1 && arr[lastRow][lastCol] != 0) {
        if (checkwin(arr, lastRow, lastCol)) {
            int winScore = (arr[lastRow][lastCol] == botPlayer ? 1000000 + depth : -1000000 - depth);
            tt_store(boardKey, depth, winScore);
            return winScore;
        }
    }

    int full = 1;
    for (int c = 0; c < 7; c++) { if (arr[0][c] == 0) { full = 0; break; } }
    if (depth == 0 || full) {
        int val = evaluate_board(arr, botPlayer);
        tt_store(boardKey, depth, val);
        return val;
    }

    int priority[7] = {3,2,4,1,5,0,6};
    int bestScore;

    if (maximizingPlayer) {
        bestScore = -100000000;
        for (int i = 0; i < 7; i++) {
            int col = priority[i]; if (arr[0][col] != 0) continue;
            int row = changefunction(arr, col, botPlayer); if (row == -1) continue;
            int score = minimax(arr, depth - 1, alpha, beta, 0, botPlayer, row, col);
            arr[row][col] = 0;
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break;
        }
    } else {
        bestScore = 100000000;
        for (int i = 0; i < 7; i++) {
            int col = priority[i]; if (arr[0][col] != 0) continue;
            int row = changefunction(arr, col, opponent); if (row == -1) continue;
            int score = minimax(arr, depth - 1, alpha, beta, 1, botPlayer, row, col);
            arr[row][col] = 0;
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;
        }
    }

    tt_store(boardKey, depth, bestScore);
    return bestScore;
}

/* Multithreaded botmove_hard: spawn one thread per legal root move (up to 7) */
int botmove_hard(int arr[6][7], int search_depth) {
    int botPlayer = 2;
    pthread_t threads[7];
    ThreadTask tasks[7];
    int priority[7] = {3,2,4,1,5,0,6};
    int tcount = 0;

    for (int i = 0; i < 7; i++) {
        int col = priority[i];
        if (arr[0][col] != 0) continue;
        memcpy(tasks[tcount].arr, arr, sizeof(tasks[tcount].arr));
        tasks[tcount].col = col;
        tasks[tcount].botPlayer = botPlayer;
        tasks[tcount].score = -100000000;
        tasks[tcount].depth = search_depth; // depth at root includes the root move
        if (pthread_create(&threads[tcount], NULL, thread_worker, &tasks[tcount]) != 0) {
            // fallback in case thread creation fails: compute synchronously
            int row = changefunction(tasks[tcount].arr, col, botPlayer);
            if (row != -1) {
                tasks[tcount].score = minimax(tasks[tcount].arr, tasks[tcount].depth, -100000000, 100000000, 0, botPlayer, row, col);
            } else {
                tasks[tcount].score = -100000000;
            }
        } else {
            tcount++;
        }
    }

    for (int i = 0; i < tcount; i++) pthread_join(threads[i], NULL);

    int bestScore = -100000000;
    int bestCol = 3;
    for (int i = 0; i < tcount; i++) {
        if (tasks[i].score > bestScore) {
            bestScore = tasks[i].score;
            bestCol = tasks[i].col;
        }
    }
    return bestCol;
}

int main() {
    // initialize empty TT keys
    for (int i=0;i<TABLE_SIZE;i++) transpositionTable[i].key[0] = '\0', transpositionTable[i].depth = -1000000;

    int arr[6][7]={0};
    int current=1, mode, difficulty=1;
    srand((unsigned)time(NULL));

    printf("Choose mode:\n1. Player vs Player\n2. Player vs Bot\n");
    if (scanf("%d",&mode) != 1) return 0;

    int humanPlayer=1, botPlayer=2;
    if (mode==2){
        printf("Choose bot difficulty:\n1. Easy\n2. Medium\n3. Hard\n");
        scanf("%d",&difficulty);
        printf("Do you want to play first or second?\n1. Play first (You = A)\n2. Play second (You = B)\n");
        int choice; scanf("%d",&choice);
        if(choice==2){ humanPlayer=2; botPlayer=1; }
        printf("\nYou are Player %c. Bot is Player %c.\n",
               (humanPlayer==1?'A':'B'), (botPlayer==1?'A':'B'));
    }

    int search_depth = MAX_DEPTH; // change if you want deeper/shallower

    while(true){
        createboard(arr);
        int col;
        if(current==humanPlayer || mode==1){
            printf("Player %c, ", current==1?'A':'B');
            col=getColumnInput();
        } else {
            if(difficulty==1) col=botmove_easy(arr);
            else if(difficulty==2) col=botmove_medium(arr,botPlayer,humanPlayer);
            else col=botmove_hard(arr, search_depth);
            printf("Bot chooses column %d\n", col+1);
        }
        if(col<0 || col>=7){ printf("Invalid column! Try again.\n"); continue; }
        int row=changefunction(arr,col,current);
        if(row==-1){ printf("Column full! Try again.\n"); continue; }
        if(checkwin(arr,row,col)){
            createboard(arr);
            if(mode==2 && current==botPlayer) printf("Bot wins!\n");
            else if(mode==2 && current==humanPlayer) printf("You win!\n");
            else printf("Player %c wins!\n", current==1?'A':'B');
            break;
        }
        current=(current==1?2:1);
    }

    return 0;
}
