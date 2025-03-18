#include "constants.h"
#include "func_defs.h"
#include "get_moves.c"
#include "search.c"
#include "helpers.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// typedef struct {
//     int num_xors;
//     unsigned long long* mov;
//     int* pc;
//     unsigned long long info;
// } Move;

char* get_bot_move(char* FEN){
    printf("%s\n",FEN);
    unsigned long long* board = from_FEN(FEN);
    Move* bot_move = root_search(board,4);
    // print_board(board);
    print_move(bot_move);
    return move_to_uci(bot_move,board);
}

int evaluate(const unsigned long long* board){
    int mg_eval = 0;
    int eg_eval = 0;
    int mg_to_eg_counter = 0;
    int pos;

    for (int pc = WHITE_PAWN; pc <= WHITE_KING; pc++){
        unsigned long long pc_board = board[pc];
        while (pc_board){
            mg_to_eg_counter += mg_to_eg_values[pc];
            pos = __builtin_ctzll(pc_board);
            mg_eval += mg_piece_table[pos+(pc<<6)];
            eg_eval += eg_piece_table[pos+(pc<<6)];
            pc_board &= (pc_board - 1);
        }
    }
    for (int pc = BLACK_PAWN; pc <= BLACK_KING; pc++){
        unsigned long long pc_board = board[pc];
        while (pc_board){
            mg_to_eg_counter += mg_to_eg_values[pc];
            pos = __builtin_ctzll(pc_board);
            mg_eval -= mg_piece_table[pos+(pc<<6)];
            eg_eval -= eg_piece_table[pos+(pc<<6)];
            pc_board &= (pc_board - 1);
        }
    }

    if (mg_to_eg_counter > 24){
        return mg_eval;
    }
    return (mg_eval * mg_to_eg_counter + eg_eval * (24 - mg_to_eg_counter)) / 24;
}

int testing(char* fen,bool print_moves){
    unsigned long long* board = from_FEN(fen);
    unsigned long long bishop = board[WHITE_BISHOP] & -board[WHITE_BISHOP];
    if (bishop == 0){
        return 0;
    }
    // unsigned long long old_attacks = get_white_bishop_attacks(board,bishop);
    unsigned long long new_attacks = new_get_white_bishop_attacks(board,bishop);
    int i = 0;
    // if (old_attacks != new_attacks && bishop){
    //     printf("===========\nCORRECT:\n");
    //     print_bit_board(old_attacks);
    //     printf("NEW:\n");
    //     print_bit_board(new_attacks);
    //     printf("ROOK\n");
    //     print_bit_board(bishop);
    //     printf("\n%s\n==================\n",fen);
    // } else {
    //     printf("MATCH");
    // }
    return 0;
}

void receiver() {
    char buffer[256];
    char FEN[256];
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0; 
        if (strncmp(buffer, "GET ", 4) == 0) {
            char message[256] = "";
            snprintf(message, sizeof(message), "%s", buffer + 4);

            if (strncmp(message, "TEST ", 5) == 0) {
                snprintf(FEN, sizeof(FEN), "%s", message + 5);
                testing(FEN, false);
                fflush(stderr);
            } 
            else if (strncmp(message, "PLAY ", 5) == 0) {
                snprintf(FEN, sizeof(FEN), "%s", message + 5);
                char* move = get_bot_move(FEN);
                fprintf(stderr,"%s\n",move);
                fflush(stderr);
            } 
            else {
                fprintf(stderr,"Unknown Message\n");
                fflush(stderr);
            }
        }
        else if (strcmp(buffer, "EXIT") == 0) {
            break;
        }
    }
}

int main() {
    receiver();
    // char* s = get_bot_move("rnbqkb1r/ppp1ppp1/5n1p/3p4/8/5NP1/PPPPPPBP/RNBQK2R w KQkq - 0 4");
    // unsigned long long *board = from_FEN("rnbqkb1r/ppp1ppp1/5n1p/3p4/8/5NP1/PPPPPPBP/RNBQK2R w KQkq - 0 4");
    // Move** movs = malloc(MOVES_ARRAY_LENGTH * sizeof(Move*));
    // Move** movptr = movs;
    // get_white_pawn_moves(&movptr,board);
    // *movptr = NULL;
    // for (Move** mov = movs; *mov != NULL; mov++){
    //     print_move(*mov);
    // }

    return 0;
}

