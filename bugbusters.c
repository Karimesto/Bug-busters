int evaluate_position(int arr[6][7], int player) {
    int score = 0;

    // Score center column (most valuable)
    int centerCount = 0;
    for (int i = 0; i < 6; i++) {
        if (arr[i][3] == player) centerCount++;
    }
    score += centerCount * 4;

    int direction[4][2] = {{0,1},{1,0},{1,1},{1,-1}};

    // Score all windows of size 4
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 7; c++) {
            for (int d = 0; d < 4; d++) {
                int dr = direction[d][0], dc = direction[d][1];
                int countPlayer = 0, countEmpty = 0;

                for (int k = 0; k < 4; k++) {
                    int rr = r + dr*k, cc = c + dc*k;
                    if (rr < 0 || rr >= 6 || cc < 0 || cc >= 7) break;
                    if (arr[rr][cc] == player) countPlayer++;
                    else if (arr[rr][cc] == 0) countEmpty++;
                }

                if (countPlayer == 3 && countEmpty == 1) score += 50;
                if (countPlayer == 2 && countEmpty == 2) score += 10;
                if (countPlayer == 1 && countEmpty == 3) score += 1;
            }
        }
    }

    return score;
}

int minimax(int arr[6][7], int depth, int alpha, int beta, int maximizingPlayer, int botPlayer) {
    // Simple depth cutoff
    if (depth == 0)
        return evaluate_position(arr, botPlayer);

    int bestScore;
    if (maximizingPlayer) {
        bestScore = -1000000;

        for (int col = 0; col < 7; col++) {
            if (arr[0][col] == 0) {
                int row = changefunction(arr, col, botPlayer);
                int score = minimax(arr, depth - 1, alpha, beta, 0, botPlayer);
                arr[row][col] = 0;

                if (score > bestScore) bestScore = score;
                if (score > alpha) alpha = score;

                if (beta <= alpha) break;
            }
        }
        return bestScore;

    } else {
        int opponent = (botPlayer == 2 ? 1 : 2);
        bestScore = 1000000;

        for (int col = 0; col < 7; col++) {
            if (arr[0][col] == 0) {
                int row = changefunction(arr, col, opponent);
                int score = minimax(arr, depth - 1, alpha, beta, 1, botPlayer);
                arr[row][col] = 0;

                if (score < bestScore) bestScore = score;
                if (score < beta) beta = score;

                if (beta <= alpha) break;
            }
        }
        return bestScore;
    }
}

int botmove_hard(int arr[6][7]) {
    int bestScore = -1000000;
    int bestCol = 3; // default center move
    int botPlayer = 2;

    for (int col = 0; col < 7; col++) {
        if (arr[0][col] == 0) {
            int row = changefunction(arr, col, botPlayer);
            int score = minimax(arr, 5, -1000000, 1000000, 0, botPlayer); 
            arr[row][col] = 0;

            if (score > bestScore) {
                bestScore = score;
                bestCol = col;
            }
        }
    }
    return bestCol;
}
