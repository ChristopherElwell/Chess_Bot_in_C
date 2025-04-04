#include "helpers.h"
#include "constants.h"
#include "search.h"
#include "get_moves.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// helper functions, many are used to print different data structures, convert encodings from one to another, freeing heap allocated memory, and various other micro-tasks


// MEMORY FREEING FUNCTIONS

void free_search_result(searchResult* sr){
    while(sr!=NULL){
        searchResult* next = sr->best_result;
        free(sr);
        sr = next;
    }
}

void free_board(uint64_t* board) {
    if (board != NULL) {
        free(board);
    }
}

// 


// PRINTING FUNCTIONS

void print_bit_board(const uint64_t b){
    for (int i = 0; i < 64; i++){
        if(i%8 == 0){
            printf("%d ",8 - i/8);
        }
        if (b & (1LL << (63 - i))){
            printf("# ");
        } else {
            printf("- ");
        }
        if ((i+1)%8 == 0){
            printf("\n");
        }
    }
    printf("  a b c d e f g h\n\n");
}

void print_board(uint64_t* const board){
    printf("------PRINTING BOARD-----\n\n");
    for (int i = WHITE_PAWN; i <= BLACK_KING; i++){
        printf("BOARD OF %s\n",PIECE_NAMES[i]);
        print_bit_board(board[i]);
        printf("\n\n");
    }
}

void print_move(Move* m){
    printf("PRINTING %s MOVE\n",MOVE_TYPES[m->type]);
    
    printf("MOVING %s\n",PIECE_NAMES[m->pc1]);
    print_bit_board(m->mov1);
    
    if (m->type == CAPTURE || m->type == PROMOTE){
        printf("MOVING %s\n",PIECE_NAMES[m->pc2]);
        print_bit_board(m->mov2);
    }
    if (m->type == CAPTURE_PROMOTE){
        printf("MOVING %s\n",PIECE_NAMES[m->pc3]);
        print_bit_board(m->mov3);
    }

    printf("INFO\n");
    print_bit_board(m->info);
    printf("\n\n");
}

void print_move_short(Move* m){
    switch(m->type){
        case EMPTY:
            printf("%s | %s\n",MOVE_TYPES[m->type],PIECE_NAMES[m->pc1]);
            return;
        case CAPTURE:
            printf("%s | %s takes %s\n",MOVE_TYPES[m->type],PIECE_NAMES[m->pc1],PIECE_NAMES[m->pc2]);
            return;
        case PROMOTE:
            printf("%s | %s promotes to %s\n",MOVE_TYPES[m->type],PIECE_NAMES[m->pc1],PIECE_NAMES[m->pc2]);
            return;
        case CAPTURE_PROMOTE:
            printf("%s | %s takes %s and promotes to %s\n",MOVE_TYPES[m->type],PIECE_NAMES[m->pc1],PIECE_NAMES[m->pc2],PIECE_NAMES[m->pc3]);
            return;
        case BOOK_END:
            printf("BOOKEND\n");
            return;
    }
}

void print_principal_variation(searchResult* sr, uint64_t* board){
    Move* princ_variation[50];
    int mov_counter = 0;
    while(sr != NULL && sr->best_move.type != BOOK_END){;
        princ_variation[mov_counter++] = &(sr->best_move);
        sr = sr->best_result;
    }
    for (int i = 0; i < mov_counter; i++){
        char* move = move_to_uci(princ_variation[i],board);
        apply_move(princ_variation[i],board);
        printf("%s,",move);
    }
    for (int i = mov_counter - 1; i >= 0; i--){
        apply_move(princ_variation[i],board);
    }
    printf("\n");
}

// APPLY, CREATE, AND COPY MOVE STRUCT FUNCTIONS
void apply_move(Move* m, uint64_t* board){
    
    board[m->pc1] ^= m->mov1;
    board[m->pc2] ^= m->mov2;
    board[m->pc3] ^= m->mov3;
    
    board[INFO] ^= m->info;
    
    board[WHITE_PCS] = board[WHITE_PAWN] |
    board[WHITE_KNIGHT] |
    board[WHITE_BISHOP] |
    board[WHITE_ROOK] |
    board[WHITE_QUEEN] |
    board[WHITE_KING]; 
    
    board[BLACK_PCS] = board[BLACK_PAWN] |
    board[BLACK_KNIGHT] |
    board[BLACK_BISHOP] |
    board[BLACK_ROOK] |
    board[BLACK_QUEEN] |
    board[BLACK_KING]; 
}

inline Move* create_move1(Move* m,int pc,uint64_t mov,uint64_t info_in){
    m->type = EMPTY;
    m->mov1 = mov;
    m->pc1 = pc;
    m->mov2 = 0;
    m->pc2 = 0;
    m->mov3 = 0;
    m->pc3 = 0;
    m->info = info_in;
    return m + 1;
}

inline Move* create_move2(Move* m,
    int pc1,uint64_t mov1,
    int pc2,uint64_t mov2,
    uint64_t info_in, movType type){
        
    m->type = type;
    m->mov1 = mov1;
    m->pc1 = pc1;
    m->mov2 = mov2;
    m->pc2 = pc2;
    m->mov3 = 0;
    m->pc3 = 0;
    m->info = info_in;
    return m + 1;
}

inline Move* create_move3(Move* m,
    int pc1,uint64_t mov1,
    int pc2,uint64_t mov2,
    int pc3,uint64_t mov3,
    uint64_t info_in){
        
        m->type = CAPTURE_PROMOTE;
        m->mov1 = mov1;
        m->pc1 = pc1;
        m->mov2 = mov2;
        m->pc2 = pc2;
        m->mov3 = mov3;
        m->pc3 = pc3;
        m->info = info_in;
        return m + 1;
    }
    
Move copy_move(const Move* original) {
    Move m;
    m.type = original->type;
    m.info = original->info;

    m.pc1 = original->pc1;
    m.pc2 = original->pc2;
    m.pc3 = original->pc3;

    m.mov1 = original->mov1;
    m.mov2 = original->mov2;
    m.mov3 = original->mov3;
    
    return m;
}

// CONVERT ENCODING TO ANOTHER

uint64_t* from_FEN(const char* p){
    uint64_t *board = calloc(BOARD_ARRAY_SIZE,sizeof(uint64_t));
    int sq = 0;

    while (*p != '\0' && *p != ' '){
        switch (*p){
            case 'P':
                board[WHITE_PAWN] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'N':
                board[WHITE_KNIGHT] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'B':
                board[WHITE_BISHOP] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'R':
                board[WHITE_ROOK] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'Q':
                board[WHITE_QUEEN] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'K':
                board[WHITE_KING] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'p':
                board[BLACK_PAWN] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'n':
                board[BLACK_KNIGHT] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'b':
                board[BLACK_BISHOP] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'r':
                board[BLACK_ROOK] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'q':
                board[BLACK_QUEEN] |= 1LL << (63 - sq);
                sq++;
                break;
            case 'k':
                board[BLACK_KING] |= 1LL << (63 - sq);
                sq++;
                break;
            case '1':
                sq++;
                break;
            case '2':
                sq += 2;
                break;
            case '3':
                sq += 3;
                break;
            case '4':
                sq += 4;
                break;
            case '5':
                sq += 5;
                break;
            case '6':
                sq += 6;
                break;
            case '7':
                sq += 7;
                break;
            case '8':
                sq += 8;
                break;
            case '/':
                break;
            case ' ':
                break;
        }
        p++;
    }

    p++;
    if(*p == 'w'){
        board[INFO] |= TURN_BIT;
    };
    p += 2;
    while(*p != ' '){
        switch (*p){
            case 'K':
                board[INFO] |= WHITE_KINGSIDE_RIGHT;
                break;
            case 'Q':
                board[INFO] |= WHITE_QUEENSIDE_RIGHT;
                break;
            case 'k':
                board[INFO] |= BLACK_KINGSIDE_RIGHT;
                break;
            case 'q':
                board[INFO] |= BLACK_QUEENSIDE_RIGHT;
                break;
        }
        p++;
    }
    p++;
    if (*p != '-'){
        char file = *p;
        p++;
        char rank = *p;
        board[INFO] |= sq_from_name(file,rank);
    }
    prep_board(board);
    return board;
}

char* move_to_uci(Move* mov, uint64_t* board){
    char *out = malloc(5);
    uint64_t starting_sq = mov->mov1 & board[mov->pc1];
    uint64_t ending_sq = mov->mov1 & ~board[mov->pc1];
    sprintf(out,"%s%s ",SQUARES[__builtin_ctzll(starting_sq)],SQUARES[__builtin_ctzll(ending_sq)]);
    switch (mov->type){
        case EMPTY:
        case CAPTURE:
            break;
        case PROMOTE:
            out[4] = PIECE_CODES[mov->pc2];
            break;
        case CAPTURE_PROMOTE:
            out[4] = PIECE_CODES[mov->pc3];
            break;
        case BOOK_END:
            out[4] = 'x';
            break;
    }
    if (out[4] == ' '){
        out[4] = out[3];
        out[3] = out[2];
        out[2] = out[1];
        out[1] = out[0];
        out[0] = ' ';
    }
    return out;
}

uint64_t sq_from_name(char file, char rank){
    return RANKS[rank - '1'] & FILES['h' - file];
}

// READ FEN CSV FOR TESTING

void read_pos_csv(const char* filename, char** FENs, int num_rows){
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file\n");
        return;
    }
    
    char line[400];
    int row = 0;
    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file) && row < num_rows) {        
        char* comma_pos = strchr(line,',');
        *comma_pos = '\0';        
        FENs[row++] = strdup(line);
    }
    
    fclose(file);
}