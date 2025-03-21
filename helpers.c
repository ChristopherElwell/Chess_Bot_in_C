#include "constants.h"
#include "func_defs.h"

void free_board(unsigned long long* board) {
    if (board != NULL) {
        free(board);
    }
}

static inline unsigned long long flipDiagA8H1(unsigned long long x) {
    unsigned long long t;
    const static unsigned long long k1 = 0x5500550055005500ULL;
    const static unsigned long long k2 = 0x3333000033330000ULL;
    const static unsigned long long k4 = 0x0f0f0f0f00000000ULL;
    t  = k4 & (x ^ (x << 28));
    x ^=       t ^ (t >> 28) ;
    t  = k2 & (x ^ (x << 14));
    x ^=       t ^ (t >> 14) ;
    t  = k1 & (x ^ (x <<  7));
    x ^=       t ^ (t >>  7) ;
    return x;
 }

unsigned long long flipDiagA1H8(unsigned long long x) {
    unsigned long long t;
    const unsigned long long k1 = 0xaa00aa00aa00aa00ULL;
    const unsigned long long k2 = 0xcccc0000cccc0000ULL;
    const unsigned long long k4 = 0xf0f0f0f00f0f0f0fULL;
    t  =       x ^ (x << 36) ;
    x ^= k4 & (t ^ (x >> 36));
    t  = k2 & (x ^ (x << 18));
    x ^=       t ^ (t >> 18) ;
    t  = k1 & (x ^ (x <<  9));
    x ^=       t ^ (t >>  9) ;
    return x;
 }

unsigned long long int sq_from_name(char file, char rank){
    return RANKS[rank - '1'] & FILES['h' - file];
}

void print_bit_board(const unsigned long long b){
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

void print_board(unsigned long long* const board){
    printf("------PRINTING BOARD-----\n\n");
    for (int i = WHITE_PAWN; i <= BLACK_KING; i++){
        printf("BOARD OF %s\n",PIECE_NAMES[i]);
        print_bit_board(board[i]);
        printf("\n\n");
    }
}

Move* copy_move(const Move* original) {
    if (!original) return NULL;
    
    switch (original->num_xors){
        case 1:
            return create_move1(original->pc1, original->mov1,original->info);
        case 2:
            return create_move2(original->pc1, original->mov1,
                                original->pc2, original->mov2,
                                original->info);
        case 3:
            return create_move3(original->pc1, original->mov1,
                                original->pc2, original->mov2,
                                original->pc3, original->mov3,
                                original->info);
    }
}

void print_move(Move* m){
    printf("PRINTING MOVE WITH %d PARTS\n",m->num_xors);
    
    printf("MOVING %s\n",PIECE_NAMES[m->pc1]);
    print_bit_board(m->mov1);
    
    if (m->num_xors >= 2){
        printf("MOVING %s\n",PIECE_NAMES[m->pc2]);
        print_bit_board(m->mov2);
    }
    if (m->num_xors == 3){
        printf("MOVING %s\n",PIECE_NAMES[m->pc3]);
        print_bit_board(m->mov3);
    }

    printf("INFO\n");
    print_bit_board(m->info);
    printf("\n\n");
}

void apply_move(Move* m, unsigned long long* board){
    
    board[m->pc1] ^= m->mov1;
    
    if (m->num_xors >= 2){
        board[m->pc2] ^= m->mov2;
    }
    if (m->num_xors >= 3){
        board[m->pc3] ^= m->mov3;
    }
    
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

Move* create_move1(int pc,unsigned long long mov,unsigned long long info_in){
    Move* m = malloc(sizeof(Move));
    if (!m) return NULL;
    m->num_xors = 1;
    m->mov1 = mov;
    m->pc1 = pc;
    m->info = info_in;
}

Move* create_move2(
    int pc1,unsigned long long mov1,
    int pc2,unsigned long long mov2,
    unsigned long long info_in){

    Move* m = malloc(sizeof(Move));
    if (!m) return NULL;
    m->num_xors = 2;
    m->mov1 = mov1;
    m->pc1 = pc1;
    m->mov2 = mov2;
    m->pc2 = pc2;
    m->info = info_in;

}

Move* create_move3(
    int pc1,unsigned long long mov1,
    int pc2,unsigned long long mov2,
    int pc3,unsigned long long mov3,
    unsigned long long info_in){

    Move* m = malloc(sizeof(Move));
    if (!m) return NULL;
    m->num_xors = 3;
    m->mov1 = mov1;
    m->pc1 = pc1;
    m->mov2 = mov2;
    m->pc2 = pc2;
    m->mov3 = mov3;
    m->pc3 = pc3;
    m->info = info_in;

}

void prep_board(unsigned long long *board){
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

unsigned long long* from_FEN(const char* p){
    unsigned long long *board = calloc(BOARD_ARRAY_SIZE,sizeof(unsigned long long));
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

char* move_to_uci(Move* mov, unsigned long long* board){
    char *out = malloc(5);
    unsigned long long starting_sq = mov->mov1 & board[mov->pc1];
    unsigned long long ending_sq = mov->mov1 & ~board[mov->pc1];
    sprintf(out,"%s%s ",SQUARES[__builtin_ctzll(starting_sq)],SQUARES[__builtin_ctzll(ending_sq)]);
    printf("MOVE: %s\n",out);
    if (mov->pc1 == WHITE_PAWN && (ending_sq & RANK_8)){
        switch (mov->pc2){
            case 1:
                out[4] = 'n';
                return out;
            case 2:
                out[4] = 'r';
                return out;
            case 3:
                out[4] = 'b';
                return out;
            case 4:
                out[4] = 'q';
                return out;
        }
        switch (mov->pc3){
            case 1:
                out[4] = 'n';
                return out;
            case 2:
                out[4] = 'r';
                return out;
            case 3:
                out[4] = 'b';
                return out;
            case 4:
                out[4] = 'q';
                return out;
        }
    } else if (mov->pc1 == BLACK_PAWN && (ending_sq & RANK_1)){
        for(int i = 1; i < mov->num_xors; i++){
            switch (mov->pc2){
                case 7:
                    out[4] = 'n';
                    return out;
                case 8:
                    out[4] = 'r';
                    return out;
                case 9:
                    out[4] = 'b';
                    return out;
                case 10:
                    out[4] = 'q';
                    return out;
            }
            switch (mov->pc3){
                case 7:
                    out[4] = 'n';
                    return out;
                case 8:
                    out[4] = 'r';
                    return out;
                case 9:
                    out[4] = 'b';
                    return out;
                case 10:
                    out[4] = 'q';
                    return out;
            }
        }
    } else {
        return out;
    }
    
}