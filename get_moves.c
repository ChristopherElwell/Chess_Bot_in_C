#include "constants.h"
#include "func_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

Move* create_move(int count, int* pcs, unsigned long long* movs, unsigned long long info_in);
void print_bit_board(const unsigned long long b);
void print_board(unsigned long long* const board);
static inline unsigned long long flipDiagA8H1(unsigned long long x);

unsigned long long get_white_rook_attacks(const unsigned long long* board, unsigned long long rook){
    static const unsigned long long diag = 0x0102040810204080; 
    static const unsigned long long anti_diag = 0x8040201008040201;
    int pos = __builtin_ctzll(rook);
    int rank = pos >> 3;
    int file = pos & 7;
    int base = pos & ~7;
    unsigned long long pcs = board[WHITE_PCS] | board[BLACK_PCS];

    unsigned long long attacks = (unsigned long long)(SLIDING_MOVES[(((pcs >> (base + 1)) & 0x3F) << 3) + file]) << base;
    
    unsigned long long file_isolated = pcs << (8 - file) & FILE_H;
    unsigned long long rotated = (file_isolated * anti_diag) >> 56;
    int index = (rotated * 8 + (7-rank)) & 0x1ff;
    unsigned long long moves_rotated = ((unsigned long long)SLIDING_MOVES[index]) * anti_diag;
    attacks |= (moves_rotated & FILE_A) >> (7 - file); 
   
    return attacks & ~board[WHITE_PCS];
}

unsigned long long get_black_rook_attacks(const unsigned long long* board, unsigned long long rook){
    static const unsigned long long diag = 0x0102040810204080; 
    static const unsigned long long anti_diag = 0x8040201008040201;
    int pos = __builtin_ctzll(rook);
    int rank = pos >> 3;
    int file = pos & 7;
    int base = pos & ~7;
    unsigned long long pcs = board[WHITE_PCS] | board[BLACK_PCS];

    unsigned long long attacks = (unsigned long long)(SLIDING_MOVES[(((pcs >> (base + 1)) & 0x3F) << 3) + file]) << base;
    
    unsigned long long file_isolated = pcs << (8 - file) & FILE_H;
    unsigned long long rotated = (file_isolated * anti_diag) >> 56;
    int index = (rotated * 8 + (7-rank)) & 0x1ff;
    unsigned long long moves_rotated = ((unsigned long long)SLIDING_MOVES[index]) * anti_diag;
    attacks |= (moves_rotated & FILE_A) >> (7 - file); 
   
    return attacks & ~board[BLACK_PCS];
}

unsigned long long old_get_white_rook_attacks(const unsigned long long* board, unsigned long long rook){
    unsigned long long sqs_ahead, temp, mask, pcs_ahead, first_pc, spots, pcs_behind;
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    const unsigned long long pcs = whites | blacks;
    
    int pos = __builtin_ctzll(rook);
    const unsigned long long file = FILES[pos & 7];
    const unsigned long long rank = RANKS[pos >> 3];
    
    // up
    sqs_ahead = ~((rook - 1) | rook);
    pcs_ahead = pcs & sqs_ahead;
    first_pc = pcs_ahead & -(pcs_ahead & file) & file;
    spots = ((first_pc - 1) & sqs_ahead & file) | (first_pc & blacks);
    
    // down
    pcs_behind = pcs & (rook - 1);
    temp = pcs_behind & file;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (rook - 1) & ~((first_pc - 1) | first_pc) & file;
    spots |= first_pc & blacks;
    spots |= file & (rook - 1) & ~mask;

    // left
    first_pc = pcs_ahead & -(pcs_ahead & rank) & rank;
    spots |= ((first_pc - 1) & sqs_ahead & rank) | first_pc & blacks;
    
    // right
    temp = pcs_behind & rank;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (rook - 1) & ~((first_pc - 1) | first_pc) & rank;
    spots |= first_pc & blacks;
    spots |= rank & (rook - 1) & ~mask;
    
    return spots;
}

unsigned long long old_get_black_rook_attacks(const unsigned long long* board, unsigned long long rook){
    unsigned long long sqs_ahead, temp, mask, pcs_ahead, first_pc, spots, pcs_behind;
    unsigned long long whites = board[WHITE_PCS];
    unsigned long long blacks = board[BLACK_PCS];
    unsigned long long pcs = whites | blacks;
    
    int pos = __builtin_ctzll(rook);
    unsigned long long file = FILES[pos & 7];
    unsigned long long rank = RANKS[pos >>3];
    
    // up
    sqs_ahead = ~((rook - 1) | rook);
    pcs_ahead = pcs & sqs_ahead;
    first_pc = pcs_ahead & -(pcs_ahead & file) & file;
    spots = ((first_pc - 1) & sqs_ahead & file) | (first_pc & whites);
    
    // down
    pcs_behind = pcs & (rook - 1);
    temp = pcs_behind & file;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (rook - 1) & ~((first_pc - 1) | first_pc) & file;
    spots |= first_pc & whites;
    spots |= file & (rook - 1) & ~mask;

    // left
    first_pc = pcs_ahead & -(pcs_ahead & rank) & rank;
    spots |= ((first_pc - 1) & sqs_ahead & rank) | (first_pc & whites);
    
    // right
    temp = pcs_behind & rank;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (rook - 1) & ~((first_pc - 1) | first_pc) & rank;
    spots |= first_pc & whites;
    spots |= rank & (rook - 1) & ~mask;

    return spots;
}

unsigned long long new_get_white_bishop_attacks(const unsigned long long* board, unsigned long long bishop){
    static const unsigned long long diag = 0x0102040810204080; 
    static const unsigned long long anti_diag = 0x8040201008040201;
    unsigned long long pcs_on_A = board[WHITE_PCS] & FILE_A;
    unsigned long long pcs_on_H = board[WHITE_PCS] & FILE_H;
    unsigned long long pcs_on_1 = board[WHITE_PCS] & RANK_1;
    unsigned long long pcs_on_8 = pcs_on_1 << 56;
    unsigned long long copied_rows = pcs_on_1 * 0x0101010101010101;
    unsigned long long triangle = copied_rows & 0xfffefdf8f0e;
} 

unsigned long long get_white_bishop_attacks(const unsigned long long* board, unsigned long long bishop){
    unsigned long long up, down, mask, temp, first_pc, spots;
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    const unsigned long long pcs = whites | blacks;
    int pos = __builtin_ctzll(bishop);
    up = DIAGS_UP[(pos&7) + (pos>>3)];
    down = DIAGS_DOWN[7 + (pos>>3) - (pos&7)];
    
    const unsigned long long sqs_ahead = ~((bishop - 1) | bishop);
    const unsigned long long pcs_ahead = pcs & sqs_ahead;
    const unsigned long long pcs_behind = pcs & (bishop - 1);
    
    // up and right
    first_pc = pcs_ahead & -(pcs_ahead & up) & up;
    spots = ((first_pc - 1) & sqs_ahead & up) | (first_pc & blacks);
    
    // down and left
    temp = pcs_behind & up;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (bishop - 1) & ~((first_pc - 1) | first_pc) & up;
    spots |= first_pc & blacks;
    spots |= up & (bishop - 1) & ~mask;

    // up and left
    first_pc = pcs_ahead & -(pcs_ahead & down) & down;
    spots |= (first_pc - 1) & sqs_ahead & down | (first_pc & blacks);
    
    // down and right
    temp = pcs_behind & down;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (bishop - 1) & ~((first_pc - 1) | first_pc) & down;
    spots |= first_pc & blacks;
    spots |= down & (bishop - 1) & ~mask;
    return spots;
}

unsigned long long get_black_bishop_attacks(const unsigned long long* board, unsigned long long bishop){

    unsigned long long up, down, mask, temp, first_pc, spots;
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    const unsigned long long pcs = whites | blacks;
    int pos = __builtin_ctzll(bishop);
    up = DIAGS_UP[(pos&7) + (pos>>3)];
    down = DIAGS_DOWN[7 + (pos>>3) - (pos&7)];
    
    const unsigned long long sqs_ahead = ~((bishop - 1) | bishop);
    const unsigned long long pcs_ahead = pcs & sqs_ahead;
    const unsigned long long pcs_behind = pcs & (bishop - 1);
    
    // up and right
    first_pc = pcs_ahead & -(pcs_ahead & up) & up;
    spots = ((first_pc - 1) & sqs_ahead & up) | (first_pc & whites);
    
    // down and left
    temp = pcs_behind & up;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (bishop - 1) & ~((first_pc - 1) | first_pc) & up;
    spots |= first_pc & whites;
    spots |= up & (bishop - 1) & ~mask;

    // up and left
    first_pc = pcs_ahead & -(pcs_ahead & down) & down;
    spots |= (first_pc - 1) & sqs_ahead & down | (first_pc & whites);
    
    // down and right
    temp = pcs_behind & down;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (bishop - 1) & ~((first_pc - 1) | first_pc) & down;
    spots |= first_pc & whites;
    spots |= down & (bishop - 1) & ~mask;
    return spots;
}

unsigned long long get_white_attackers(const unsigned long long* board){
    unsigned long long attacks = 0;
    attacks |= ((board[WHITE_PAWN] & ~FILE_H) << 7) | ((board[WHITE_PAWN] & ~FILE_A) << 9);

    for (unsigned long long pcs = board[WHITE_KING]; pcs; pcs &= (pcs - 1)){
        attacks |= KING_MOVES[__builtin_ctzll(pcs & -pcs)];
    }
    
    for (unsigned long long pcs = board[WHITE_KNIGHT]; pcs; pcs &= (pcs - 1)){
        attacks |= KNIGHT_MOVES[__builtin_ctzll(pcs & -pcs)];
    }
    
    for (unsigned long long pcs = (board[WHITE_BISHOP] | board[WHITE_QUEEN]); pcs; pcs &= (pcs - 1)){
        attacks |= get_white_bishop_attacks(board,pcs & -pcs);
    }
    
    for (unsigned long long pcs = (board[WHITE_ROOK] | board[WHITE_QUEEN]); pcs; pcs &= (pcs - 1)){
        attacks |= get_white_rook_attacks(board,pcs & -pcs);
    }

    return attacks;
}

unsigned long long get_black_attackers(const unsigned long long* board){

    unsigned long long attacks = 0;
    
    attacks |= ((board[BLACK_PAWN] & ~FILE_H) >> 9) | ((board[BLACK_PAWN] & ~FILE_A) >> 7);
    
    for (unsigned long long pcs = board[BLACK_KING]; pcs; pcs &= (pcs - 1)){
        attacks |= KING_MOVES[__builtin_ctzll(pcs & -pcs)];
    }
    
    for (unsigned long long pcs = board[BLACK_KNIGHT]; pcs; pcs &= (pcs - 1)){
        attacks |= KNIGHT_MOVES[__builtin_ctzll(pcs & -pcs)];
    }
    
    for (unsigned long long pcs = (board[BLACK_BISHOP] | board[BLACK_QUEEN]); pcs; pcs &= (pcs - 1)){
        attacks |= get_black_bishop_attacks(board,pcs & -pcs);
    }
    
    for (unsigned long long pcs = (board[BLACK_ROOK] | board[BLACK_QUEEN]); pcs; pcs &= (pcs - 1)){
        attacks |= get_black_rook_attacks(board,pcs & -pcs);
        // print_bit_board(get_black_rook_attacks(board,pcs & -pcs));
    }

    return attacks;
}

void get_white_knight_moves(Move*** movptr,const unsigned long long* board){

    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];

    int white_knight = WHITE_KNIGHT;
    unsigned long long knights = board[WHITE_KNIGHT];
    unsigned long long knight, taking_move, taken_piece;

    unsigned long long m[2];
    int p[] = {white_knight,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; knights; knights &= (knights - 1)){
        knight = knights & -knights;
        unsigned long long moves = KNIGHT_MOVES[__builtin_ctzll(knight)] & ~whites;
        for (unsigned long long clear_moves = moves & ~blacks; clear_moves; clear_moves &= (clear_moves - 1)){
            unsigned long long clear_move = (clear_moves & -clear_moves) | knight;
            *(*movptr)++ = create_move(1,&white_knight,&clear_move,info_xor); 
        }
        for (unsigned long long taking_moves = moves & blacks; taking_moves; taking_moves &= (taking_moves - 1)){
            taking_move = (taking_moves & -taking_moves) | knight;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if (taken_piece = (taking_move & board[pc])){
                    m[0] = taking_move;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }        
        }
    }
}

void get_black_knight_moves(Move*** movptr, const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long knights = board[BLACK_KNIGHT];
    unsigned long long knight, taking_move, taken_piece;
    int black_knight = BLACK_KNIGHT;

    unsigned long long m[2];
    int p[] = {black_knight,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; knights; knights &= (knights - 1)){
        knight = knights & -knights;
        unsigned long long moves = KNIGHT_MOVES[__builtin_ctzll(knight)] & ~blacks;
        for (unsigned long long clear_moves = moves & ~whites; clear_moves; clear_moves &= (clear_moves - 1)){
            unsigned long long clear_move = (clear_moves & (~clear_moves + 1)) | knight;
            *(*movptr)++ = create_move(1,&black_knight,&clear_move,info_xor); 
        }
        for (unsigned long long taking_moves = moves & whites; taking_moves; taking_moves &= (taking_moves - 1)){
            taking_move = (taking_moves & (~taking_moves + 1)) | knight;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if (taken_piece = (taking_move & board[pc])){
                    m[0] = taking_move,
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }        
        }
    }
}

void get_white_rook_moves(Move*** movptr, const unsigned long long* board){
    unsigned long long rooks = board[WHITE_ROOK];
    int white_rook = WHITE_ROOK;
    unsigned long long pcs = board[WHITE_PCS] | board[BLACK_PCS];
    unsigned long long rook, spot, taken_piece; 
    unsigned long long m[2];
    int p[] = {white_rook,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for(;rooks; rooks &= (rooks - 1)){
        rook = rooks & -rooks;
        unsigned long long moves = get_white_rook_attacks(board,rook);
        unsigned long long spots = moves & ~board[BLACK_PCS];
        unsigned long long taking_spots = moves & board[BLACK_PCS];
        // empty moves
        unsigned long long castling_rights_info_xor = info_xor | (board[INFO] & rook & (WHITE_KINGSIDE_RIGHT | WHITE_QUEENSIDE_RIGHT));
        for(; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | rook;
            *(*movptr)++ = create_move(1,&white_rook,&spot,castling_rights_info_xor);
        }
        
        // taking moves
        for(;taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | rook;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,castling_rights_info_xor);
                    break;
                }
            }
        }
    }
}

void get_black_rook_moves(Move*** movptr, const unsigned long long* board){
    unsigned long long rooks = board[BLACK_ROOK];
    int black_rook = BLACK_ROOK;
    unsigned long long pcs = board[WHITE_PCS] | board[BLACK_PCS];
    unsigned long long rook, spot, taken_piece; 
    unsigned long long m[2];
    int p[] = {black_rook,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for(;rooks; rooks &= (rooks - 1)){
        rook = rooks & -rooks;
        unsigned long long moves = get_black_rook_attacks(board,rook);
        unsigned long long castling_rights_info_xor = info_xor | (board[INFO] & rook & (BLACK_KINGSIDE_RIGHT | BLACK_QUEENSIDE_RIGHT)); 
        // empty moves
        for(unsigned long long spots = moves & ~board[WHITE_PCS]; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | rook;
            *(*movptr)++ = create_move(1,&black_rook,&spot,castling_rights_info_xor);
        }
        
        // taking moves
        for(unsigned long long taking_spots = moves & board[WHITE_PCS]; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | rook;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,castling_rights_info_xor);
                    break;
                }
            }
        }
    }
    }

void get_white_bishop_moves(Move*** movptr, const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long bishops = board[WHITE_BISHOP];
    unsigned long long pcs = whites | blacks;
    int white_bishop = WHITE_BISHOP;
    unsigned long long bishop, spot, taken_piece;
    
    unsigned long long m[2];
    int p[] = {white_bishop,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; bishops; bishops &= (bishops - 1)){
        bishop = bishops & -bishops;
        const unsigned long long moves = get_white_bishop_attacks(board,bishop);
        // empty moves
        for(unsigned long long spots = moves & ~blacks; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | bishop;
            *(*movptr)++ = create_move(1,&white_bishop,&spot,info_xor);
        }
        
        // taking moves
        for(unsigned long long taking_spots = moves & blacks; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | bishop;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }
        }
    }
}

void get_black_bishop_moves(Move*** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    unsigned long long bishops = board[BLACK_BISHOP];
    int black_bishop = BLACK_BISHOP;
    unsigned long long bishop, spot, taken_piece;
    
    unsigned long long m[2];
    int p[] = {black_bishop,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; bishops; bishops &= (bishops - 1)){
        bishop = bishops & -bishops;
        const unsigned long long moves = get_black_bishop_attacks(board,bishop);
        
        for(unsigned long long spots = moves & ~whites; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | bishop;
            *(*movptr)++ = create_move(1,&black_bishop,&spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & whites; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | bishop;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }
        }
    }
}

void get_white_pawn_moves(Move*** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long pawns = board[WHITE_PAWN];
    unsigned long long pcs = whites | blacks;
    unsigned long long spot, taken_piece, promotion_sq;
    int white_pawn = WHITE_PAWN;
    int black_pawn = BLACK_PAWN;

    unsigned long long m[2];
    unsigned long long m3[3];
    int p[] = {white_pawn,-1};
    int p3[] = {white_pawn,-1,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    // step forward one
    for (unsigned long long one_step = (pawns << 8) & ~pcs; one_step; one_step &= (one_step - 1)){
        spot = (one_step & (~one_step + 1));
        spot |= spot >> 8;
        if (promotion_sq = (spot & RANK_8)){
            m[0] = spot;
            m[1] = promotion_sq;
            p[1] = WHITE_KNIGHT;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = WHITE_BISHOP;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = WHITE_ROOK;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = WHITE_QUEEN;
            *(*movptr)++ = create_move(2,p,m,info_xor);
        } else {
            *(*movptr)++ = create_move(1,&white_pawn,&spot,info_xor);
        }
    }
    // step forward two
    for (unsigned long long two_step = ((pawns & RANK_2) << 16) & ~((pcs) | (pcs << 8)); two_step; two_step &= (two_step - 1)){
        spot = (two_step & (~two_step + 1));
        unsigned long long info_xor_with_enpassent = (spot >> 8) | info_xor; // creating en passent
        spot |= spot >> 16;
        *(*movptr)++ = create_move(1,&white_pawn,&spot,info_xor_with_enpassent);
    }

    // taking right
    for (unsigned long long take_right = (pawns << 7) & blacks & ~FILE_A; take_right; take_right &= (take_right - 1)){
        spot = (take_right & (~take_right + 1));
        spot |= spot >> 7;
        for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
            if (taken_piece = (spot & board[pc])){
                if (promotion_sq = (spot & RANK_8)){
                    m3[0] = spot;
                    m3[1] = taken_piece;
                    m3[2] = promotion_sq;
                    p3[1] = pc;
                    p3[2] = WHITE_QUEEN;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_KNIGHT;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_BISHOP;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_ROOK;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                } else {
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                }
                break;
            }
        }
    }
    // taking left
    for (unsigned long long take_left = (pawns << 9) & blacks & ~FILE_H; take_left; take_left &= (take_left - 1)){
        spot = (take_left & (~take_left + 1));
        spot |= spot >> 9;
        for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
            if (taken_piece = (spot & board[pc])){
                if (promotion_sq = (spot & RANK_8)){
                    m3[0] = spot;
                    m3[1] = taken_piece;
                    m3[2] = promotion_sq;
                    p3[1] = pc;
                    p3[2] = WHITE_QUEEN;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_KNIGHT;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_BISHOP;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = WHITE_ROOK;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                } else {
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                }
                break;
            }
        }
    }
    // taking en passent 
    unsigned long long en_passent_take_left = ((pawns << 9) & board[INFO] & ~RANK_1);
    p[1] = black_pawn;
    if (en_passent_take_left){
        m[0] = en_passent_take_left | (en_passent_take_left >> 9);
        m[1] = en_passent_take_left >> 8;
        *(*movptr)++ = create_move(2,p,m,info_xor);
    }
    
    unsigned long long en_passent_take_right = ((pawns << 7) & board[INFO] & ~RANK_1);
    if (en_passent_take_right){
        m[0] = en_passent_take_right | (en_passent_take_right >> 7);
        m[1] = en_passent_take_right >> 8;
        *(*movptr)++ = create_move(2,p,m,info_xor);
    }

}

void get_black_pawn_moves(Move*** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long pawns = board[BLACK_PAWN];
    unsigned long long pcs = whites | blacks;
    unsigned long long spot, taken_piece, promotion_sq;
    int black_pawn = BLACK_PAWN;
    int white_pawn = BLACK_PAWN;

    unsigned long long m[2];
    unsigned long long m3[3];
    int p[] = {black_pawn,-1};
    int p3[] = {black_pawn,-1,-1};
    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (unsigned long long one_step = (pawns >> 8) & ~pcs; one_step; one_step &= (one_step - 1)){
        spot = (one_step & (~one_step + 1));
        spot |= spot << 8;
        if (promotion_sq = (spot & RANK_1)){
            m[0] = spot;
            m[1] = promotion_sq;
            p[1] = BLACK_KNIGHT;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = BLACK_BISHOP;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = BLACK_ROOK;
            *(*movptr)++ = create_move(2,p,m,info_xor);
            p[1] = BLACK_QUEEN;
            *(*movptr)++ = create_move(2,p,m,info_xor);
        } else {
            *(*movptr)++ = create_move(1,&white_pawn,&spot,info_xor);
        }
    }
    for (unsigned long long two_step = ((pawns & RANK_7) >> 16) & ~((pcs) | (pcs >> 8)); two_step; two_step &= (two_step - 1)){
        spot = (two_step & (~two_step + 1));
        unsigned long long info_xor_with_enpassent = (spot << 8) | info_xor;
        spot |= spot << 16;
        *(*movptr)++ = create_move(1,&black_pawn,&spot,info_xor_with_enpassent);
    }
    for (unsigned long long take_right = (pawns >> 9) & whites & ~FILE_A; take_right; take_right &= (take_right - 1)){
        spot = (take_right & (~take_right + 1));
        spot |= spot << 9;
        for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
            if (taken_piece = (spot & board[pc])){
                if (promotion_sq = (spot & RANK_1)){
                    m3[0] = spot;
                    m3[1] = taken_piece;
                    m3[2] = promotion_sq;
                    p3[1] = pc;
                    p3[2] = BLACK_QUEEN;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_KNIGHT;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_BISHOP;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_ROOK;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                } else {
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                }
                break;
            }
        }
    }
    for (unsigned long long take_left = (pawns >> 7) & whites & ~FILE_H; take_left; take_left &= (take_left - 1)){
        spot = (take_left & (~take_left + 1));
        spot |= spot << 7;
        for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
            if (taken_piece = (spot & board[pc])){
                if (promotion_sq = (spot & RANK_1)){
                    m3[0] = spot;
                    m3[1] = taken_piece;
                    m3[2] = promotion_sq;
                    p3[1] = pc;
                    p3[2] = BLACK_QUEEN;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_KNIGHT;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_BISHOP;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                    p3[2] = BLACK_ROOK;
                    *(*movptr)++ = create_move(3,p3,m3,info_xor);
                } else {
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                }
                break;
            }
        }
    }

    // taking en passent 
    unsigned long long en_passent_take_left = ((pawns >> 7) & board[INFO] & ~RANK_1);
    p[1] = black_pawn;
    if (en_passent_take_left){
        m[0] = en_passent_take_left | (en_passent_take_left << 7);
        m[1] = en_passent_take_left << 8;
        *(*movptr)++ = create_move(2,p,m,info_xor);
    }
    
    unsigned long long en_passent_take_right = ((pawns >> 9) & board[INFO] & ~RANK_1);
    if (en_passent_take_right){
        m[0] = en_passent_take_right | (en_passent_take_right << 9);
        m[1] = en_passent_take_right << 8;
        *(*movptr)++ = create_move(2,p,m,info_xor);
    }
}

void get_white_king_moves(Move*** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long king = board[WHITE_KING];
    unsigned long long moves = KING_MOVES[__builtin_ctzll(king)] & ~whites;
    unsigned long long move, taken_piece;
    int white_king = WHITE_KING;

    const unsigned long long info_xor = TURN_BIT | ((~RANK_8) & board[INFO]);

    unsigned long long m[2];
    int p[] = {white_king,-1};
    for(unsigned long long clear_moves = moves & ~blacks; clear_moves; clear_moves &= (clear_moves - 1)){
        move = (clear_moves & (-clear_moves)) | king;
        *(*movptr)++ = create_move(1, &white_king, &move, info_xor);
    }
    
    for(unsigned long long taking_moves = moves & blacks; taking_moves; taking_moves &= (taking_moves - 1)){
        move = (taking_moves & (-taking_moves)) | king;
        for (int pc = BLACK_PAWN; pc < BLACK_KING; pc++){
            if (taken_piece = board[pc] & move){
                m[0] = move;
                m[1] = taken_piece;
                p[1] = pc;
                *(*movptr)++ = create_move(2,p,m, info_xor);
            }
        }        
    } 
    unsigned long long attacks = 0;
    if ((board[INFO] & WHITE_KINGSIDE_RIGHT) && 
    ((WHITE_KINGSIDE_SPACE & (whites | blacks)) == 0) &&
    (board[WHITE_ROOK] & FILE_H & RANK_1)){
        attacks = get_white_attackers(board);
        if ((attacks & WHITE_KINGSIDE_ATTACKED) == 0){
            m[0] = 0b1010ULL;
            m[1] = 0b0101ULL;
            p[1] = WHITE_ROOK;
            *(*movptr)++ = create_move(2,p,m, info_xor);
        }
    }
    if ((board[INFO] & WHITE_QUEENSIDE_RIGHT) && 
        ((WHITE_QUEENSIDE_SPACE & (whites | blacks)) == 0) &&
        ((board[WHITE_ROOK] & FILE_A & RANK_1))){
            if (attacks == 0) attacks = get_white_attackers(board);
            
            if ((attacks & WHITE_QUEENSIDE_ATTACKED) == 0){
                m[0] = 0b101000ULL;
                m[1] = 0b10010000ULL;
                p[1] = WHITE_ROOK;
                *(*movptr)++ = create_move(2,p,m, info_xor);
            }
    }

}

void get_black_king_moves(Move*** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long king = board[BLACK_KING];
    unsigned long long moves = KING_MOVES[__builtin_ctzll(king)] & ~blacks;
    unsigned long long move, taken_piece;
    int black_king = BLACK_KING;

    const unsigned long long info_xor = TURN_BIT | ((~RANK_1) & board[INFO]);

    unsigned long long m[2];
    int p[] = {black_king,-1};
    for(unsigned long long clear_moves = moves & ~whites; clear_moves; clear_moves &= clear_moves - 1){
        move = (clear_moves & (-clear_moves)) | king;
        *(*movptr)++ = create_move(1,&black_king,&move,info_xor);
    }

    for(unsigned long long taking_moves = moves & whites; taking_moves; taking_moves &= taking_moves - 1){
        move = (taking_moves & (-taking_moves)) | king;
        for (int pc = WHITE_PAWN; pc < WHITE_KING; pc++){
            if (taken_piece = board[pc] & move){
                m[0] = move;
                m[1] = taken_piece;
                p[1] = pc;
                *(*movptr)++ = create_move(2,p,m,info_xor);
            }
        }        
    }
    unsigned long long attacks;
    if ((board[INFO] & BLACK_KINGSIDE_RIGHT) && 
        ((BLACK_KINGSIDE_SPACE & (whites | blacks)) == 0) &&
        (board[BLACK_ROOK] & FILE_H & RANK_8)){
            attacks = get_black_attackers(board);
            if ((attacks & BLACK_KINGSIDE_ATTACKED) == 0){
                m[0] = 0b1010ULL << 56;
                m[1] = 0b0101ULL << 56;
                p[1] = BLACK_ROOK;
                *(*movptr)++ = create_move(2,p,m,info_xor);
            }
    }

    if ((board[INFO] & BLACK_QUEENSIDE_RIGHT) && 
        ((BLACK_QUEENSIDE_SPACE & (whites | blacks)) == 0) &&
        (board[BLACK_ROOK] & FILE_A & RANK_8)){
            if (attacks == 0) attacks = get_black_attackers(board);
            
            if ((attacks & BLACK_QUEENSIDE_ATTACKED) == 0){
                m[0] = 0b101000ULL << 56;
                m[1] = 0b10010000ULL << 56;
                p[1] = BLACK_ROOK;
                *(*movptr)++ = create_move(2,p,m,info_xor);
            }
    }
}

void get_white_queen_moves(Move*** movptr, const unsigned long long* board){
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long queens = board[WHITE_QUEEN];
    int white_queen = WHITE_QUEEN;
    unsigned long long queen, spot, taken_piece;
    
    unsigned long long m[2];
    int p[] = {white_queen,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; queens; queens &= (queens - 1)){
        queen = queens & -queens;
        const unsigned long long moves = get_white_bishop_attacks(board,queen) | get_white_rook_attacks(board,queen);
        
        for(unsigned long long spots = moves & ~blacks; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | queen;
            *(*movptr)++ = create_move(1,&white_queen,&spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & blacks; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | queen;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }
        }
    }
}

void get_black_queen_moves(Move*** movptr, const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    unsigned long long queens = board[BLACK_QUEEN];
    int white_queen = BLACK_QUEEN;
    unsigned long long queen, spot, taken_piece;
    
    unsigned long long m[2];
    int p[] = {white_queen,-1};

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; queens; queens &= (queens - 1)){
        queen = queens & -queens;
        const unsigned long long moves = get_black_bishop_attacks(board,queen) | get_black_rook_attacks(board,queen);
        
        for(unsigned long long spots = moves & ~whites; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | queen;
            *(*movptr)++ = create_move(1,&white_queen,&spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & whites; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | queen;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if (taken_piece = (spot & board[pc])){
                    m[0] = spot;
                    m[1] = taken_piece;
                    p[1] = pc;
                    *(*movptr)++ = create_move(2,p,m,info_xor);
                    break;
                }
            }
        }
    }
}

Move** get_white_moves(const unsigned long long* board){
    Move** movs = malloc(MOVES_ARRAY_LENGTH * sizeof(Move*));
    if (!movs) return NULL;

    Move** movptr = movs;
    get_white_queen_moves(&movptr,board);
    get_white_rook_moves(&movptr,board);
    get_white_bishop_moves(&movptr,board);
    get_white_knight_moves(&movptr,board);
    get_white_pawn_moves(&movptr,board);
    get_white_king_moves(&movptr,board);
    *movptr = NULL;
    return movs;
}

Move** get_black_moves(const unsigned long long* board){
    Move** movs = malloc(MOVES_ARRAY_LENGTH * sizeof(Move*));
    if (!movs) return NULL;

    Move** movptr = movs;
    get_black_queen_moves(&movptr,board);
    get_black_rook_moves(&movptr,board);
    get_black_bishop_moves(&movptr,board);
    get_black_knight_moves(&movptr,board);
    get_black_pawn_moves(&movptr,board);
    get_black_king_moves(&movptr,board);
    *movptr = NULL;
    return movs;
}