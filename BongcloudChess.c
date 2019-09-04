#include <stdlib.h>
#include <string.h>
#include <time.h>

#define max(a,b) (((a) > (b)) ? (a) : (b))

typedef struct B_board{
    char (*board)[8][8];
    int ply;
    int kings[2];
} B_board;

typedef struct move{
    int from; // could store both in same int and use masking
    int to;
} move;

typedef struct movearray{
    size_t size;
    move moves[24];
} movearray;

B_board * mainBoard;

void init_board() {
    mainBoard = malloc(sizeof(B_board));
    srand((unsigned)time(NULL));
    static char initBoard[8][8] = {
                    {'.', '.', '.', '.', '.', '.', '.', '.'},
                    {'p', 'p', 'p', 'p', '.', 'p', 'p', 'p'},
                    {'.', '.', '.', '.', 'k', '.', '.', '.'},
                    {'.', '.', '.', '.', 'p', '.', '.', '.'},
                    {'.', '.', '.', '.', 'P', '.', '.', '.'},
                    {'.', '.', '.', '.', 'K', '.', '.', '.'},
                    {'P', 'P', 'P', 'P', '.', 'P', 'P', 'P'},
                    {'.', '.', '.', '.', '.', '.', '.', '.'}};
    //memcpy(&board, &initBoard, sizeof(initBoard));
    
    mainBoard->board = &initBoard;
    mainBoard->ply = 1;
    mainBoard->kings[0] = 6;
    mainBoard->kings[1] = 2;
}

void make_move(move move, B_board *Bboard) {
    char piece = ((char*)(Bboard -> board))[move.from];
    char takePiece = ((char*)(Bboard -> board))[move.to];

    if (piece == 'K') { Bboard->kings[0] = move.to / 8; } 
    else if (piece == 'k') { Bboard->kings[1] = move.to / 8; }

    if (takePiece == 'K') { Bboard->kings[0] = -1; } 
    else if (takePiece == 'k') { Bboard->kings[1] = -1; }

    ((char*)(Bboard -> board))[move.from] = '.';
    ((char*)(Bboard -> board))[move.to] = piece;
    Bboard->ply++;
}

void get_moves(const B_board *Bboard, movearray *moveArray) {
    char (*board)[8][8] = Bboard->board;
    moveArray->size = 0;
    
    int is_white = Bboard->ply % 2;
    int pawn_direction = -(is_white*2-1);
    int row, col;
    for (int i = 1; i < 7; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = (*board)[i][j];
            if (piece != '.' && is_white == (piece < 'a')) {
                #define addtomoves() moveArray->moves[moveArray->size].from = i*8+j;\
                            moveArray->moves[moveArray->size].to = row*8+col;\
                            moveArray->size++

                if (piece == 'p' || piece == 'P') {
                    if ((!is_white && i == 1)||(is_white && i == 6)) {
                        row = i + pawn_direction * 2;
                        col = j;
                        if ((*board)[row][col] == '.' && (*board)[i+pawn_direction][col] == '.') {
                            addtomoves();
                        }
                    }
                    int directions[2][2] = {{pawn_direction, 1}, {pawn_direction, -1}};
                    for (size_t k = 0; k < 2; k++) {
                        row = directions[k][0] + i;
                        col = directions[k][1] + j;
                        if (col < 8 && col >= 0 && (*board)[row][col] != '.' && is_white != ((*board)[row][col] < 'a')) {
                            addtomoves();
                        }
                    }
                    row = i + pawn_direction;
                    col = j;
                    if ((*board)[row][col] == '.') {
                        addtomoves();
                    }
                }
                else {
                    int directions[5][2] = {{0, 1}, {0, -1}, {pawn_direction, 0}, {pawn_direction, 1}, {pawn_direction, -1}};

                    for (size_t k = 0; k < 5; k++)
                    {
                        row = directions[k][0] + i;
                        col = directions[k][1] + j;

                        if (col < 8 && col >= 0 && ((*board)[row][col] == '.'  || is_white != ((*board)[row][col] < 'a'))) {
                            addtomoves();
                        }
                    }
                }
                #undef addtomoves
            }
        }
    }
}

void get_squares(int square, int * output) {
    movearray moveArray;
    get_moves(mainBoard, &moveArray);
    int count = 0;
    for (size_t i = 0; i < moveArray.size; i++)
    {
        if (square == moveArray.moves[i].from) {
            output[count] = moveArray.moves[i].to;
            count++;
        }
    }
    for (size_t i = count; i < 5; i++)
    {
        output[i] = -1;
    }
}
int is_terminal(int kings[2]) {
    if (kings[0] == -1 || kings[1] == -1 || kings[0] == 0  || kings[1] == 7) {
        return 1;
    }
    return 0;
}

int score_board(int kings[2]) {
    if (kings[1] == -1 || (kings[0] == 0 && kings[1] != 6)) {
        return 1;
    } else if (kings[0] == -1 || kings[1] == 7) {
        return -1;
    }
    return 0;
}

B_board * get_bboardcpy(B_board * Bboard) {
    B_board * bboardcpy = malloc(sizeof(B_board));
    char (*board)[8][8] = malloc(sizeof(*Bboard->board));
    memcpy(board, Bboard->board, sizeof(*Bboard->board));
    bboardcpy->board = board;
    bboardcpy->kings[0] = Bboard->kings[0];
    bboardcpy->kings[1] = Bboard->kings[1];
    bboardcpy->ply = Bboard->ply;
    return bboardcpy;
}

int negamax(B_board *Bboard, int depth) {
    if (depth == 0 || is_terminal(Bboard->kings)) {
        return (Bboard->ply%2*2-1)*score_board(Bboard->kings);
    }

    int value = -2;

    movearray moveArray;
    get_moves(Bboard, &moveArray);
    
    for (size_t i = 0; i < moveArray.size; i++)
    {
        B_board * bboardcpy = get_bboardcpy(Bboard);
        make_move(moveArray.moves[i], bboardcpy);
        
        int tmpValue = -negamax(bboardcpy, depth-1);
        free(bboardcpy->board);
        free(bboardcpy);
        if (tmpValue == 1) { return 1; }
        value = max(value, tmpValue);
    }
    return value;
}

void ai_move(int depth) {
    B_board *Bboard = mainBoard; 
    move bestMove;
    int foundMove = 0;

    movearray moveArray;
    movearray goodMoveArray;
    goodMoveArray.size = 0;
    get_moves(Bboard, &moveArray);

    int tempValue;
    for (size_t i = 0; i < moveArray.size; i++)
    {
        B_board * bboardcpy = get_bboardcpy(Bboard);
        make_move(moveArray.moves[i], bboardcpy);
        
        tempValue = -negamax(bboardcpy, depth-1);

        free(bboardcpy->board);
        free(bboardcpy);

        if (tempValue == 0) {
            goodMoveArray.moves[goodMoveArray.size] = moveArray.moves[i];
            goodMoveArray.size++;
        } else if (tempValue == 1) {
            bestMove = moveArray.moves[i];
            foundMove++;
            break;
        }
    }
    
    if (!foundMove) { bestMove = (goodMoveArray.size != 0) ? goodMoveArray.moves[rand() % goodMoveArray.size] : moveArray.moves[rand() % moveArray.size];}
    make_move(bestMove, Bboard);
}

int user_move(int from, int to) {
    move move = {from, to};
    movearray moveArray;
    get_moves(mainBoard, &moveArray);
    int validMove = 0;
    for (size_t i = 0; i < moveArray.size; i++)
    {
        if (moveArray.moves[i].from == move.from && moveArray.moves[i].to == move.to ) {
            validMove = 1;
            make_move(move, mainBoard);
            break;
        }
    }
    return validMove;
}

int is_mainboard_terminal() { return is_terminal(mainBoard->kings); }

int score_mainboard() { return score_board(mainBoard->kings); }

char* get_board() {
    return (char*)mainBoard->board;
}