#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void init_board(B_board * mainBoard) {
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
    //move * moves = (move*) malloc(sizeof(move)*20);
    //size_t index = 0;
    
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
                            //moves[index] = malloc(sizeof(move));
                            addtomoves();
                        }
                    }
                }
                #undef addtomoves
            }
        }
    }
    //realloc(moves, sizeof(move)*index);
    //free(moves);
}

int is_terminal(int kings[2]) {
    /*int kings[2] = {-1, -1}; //maybe store kings within board as to not repeat searching
    
    unsigned long index = 0;
    while ((kings[0] == -1 || kings[1] == -1) && index < 64) {
        char * piece = (board + (size_t)index);
        if (*piece == 'K') {
            kings[0] = index / 8;
        } else if (*piece == 'k') {
            kings[1] = index / 8;;
        }
        index++;
    }*/
    if (kings[0] == -1 || kings[1] == -1 || kings[0] == 0  || kings[1] == 7) {
        return 1;
    }
    return 0;
}

int score_board(int kings[2]) {
    /*int score = 0;
    int kings[2] = {-1, -1};
    
    unsigned long index = 0;
    while ((kings[0] == -1 || kings[1] == -1) && index < 64) {
        
        char * piece = (board + (size_t)index);

        #define upscke(zo) score += (*piece < 'a')*2-1; kings[zo] = index / 8
        if (*piece == 'K') {
            upscke(0);
        } else if (*piece == 'k') {
            upscke(1);
        }
        #undef upscke
        index++;
    }*/
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

void tree(int depth, B_board *Bboard, move move, unsigned long long * num) {
    *num = *num + 1;
    if (depth == 0 || is_terminal(Bboard->kings)) { return; }

    B_board * bboardcpy = get_bboardcpy(Bboard);
    make_move(move, bboardcpy);

    movearray moveArray;
    get_moves(bboardcpy, &moveArray);
    
    for (size_t i = 0; i < moveArray.size; i++)
    {
        tree(depth - 1, bboardcpy, moveArray.moves[i], num);
    }
    free(bboardcpy->board);
    free(bboardcpy);
    return;
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

move ai_move(B_board *Bboard, int depth) {
    move bestMove;
    //int noBadMoves = 1;
    //int bestValue = -1;

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
            return moveArray.moves[i];
        }
    }
    
    return (goodMoveArray.size != 0) ? goodMoveArray.moves[rand() % goodMoveArray.size] : moveArray.moves[rand() % moveArray.size];
}

B_board * mainBoard;

char* get_board() {
    return (char*)mainBoard->board;
}

void print_board(B_board * Bboard) {
    printf("\n"); 
    for (int i=0; i < 8; i++) {
        printf("%d  ", i);

        for (int j = 0; j < 8; j++) {
            printf("%c ", (*Bboard->board)[i][j]);
        }
        printf("\n"); 
    }
    printf("   ");
    for (int j = 0; j < 8; j++) {
        printf("%c ", (char)('a'+j));
    }
    printf("\n"); 
}

move get_move_from_input(char input[6]) {
    int from = 8*(input[1] - '0') + input[0] - 'a';
    int to = 8*(input[3] - '0') + input[2] - 'a';
    move move = {from, to};
    return move;
}
 
int main(){
    mainBoard = malloc(sizeof(B_board));
    init_board(mainBoard);

    srand((unsigned)time(NULL));
    /* Print number of nodes at depth 6
    move nullmove = {0, 0};
    mainBoard->ply--;
    unsigned long long num = 0;
    tree(6, mainBoard, nullmove, &num);
    printf("%llu\n", num-1);
    */
    printf("--------------- Bongcloud Chess ---------------\n");
    char input[6];
    move move;
    while (1) {
        print_board(mainBoard);
        
        int validMove = 0;
        while (!validMove) {
            printf("Enter a move > ");
            fgets(input, 6, stdin);
            move = get_move_from_input(input);
            fflush(stdin);

            movearray moveArray;
            get_moves(mainBoard, &moveArray);
            for (int i = 0; i < moveArray.size; i++) {
                if (moveArray.moves[i].from == move.from && moveArray.moves[i].to == move.to) {
                    validMove = 1;
                }
            }
            if (!validMove) {
                printf("That move is invalid\n");
            }
        }

        make_move(move, mainBoard);
        if (is_terminal(mainBoard->kings)) {break;}

        move = ai_move(mainBoard, 6);

        make_move(move, mainBoard);
        if (is_terminal(mainBoard->kings)) {break;}
        
    }
    
    int score = score_board(mainBoard->kings);
    if (score == 1) {
        printf("White wins!\n");
    } else if (score == -1) {
        printf("Black wins!\n");
    } else {
        printf("Draw!\n");
    }
    return 0;
}