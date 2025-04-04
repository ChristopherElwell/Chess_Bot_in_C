#include "eval.h"
#include "constants.h"


int16_t evaluate(const unsigned long long* board){
    int16_t mg_eval = 0;
    int16_t eg_eval = 0;
    int16_t mg_to_eg_counter = 0;
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