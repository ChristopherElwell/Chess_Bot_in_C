#include "get_moves.h"
#include "constants.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// get_moves.c does what it says, provides an array of legal moves given a position. get_white_moves, get_black_moves, get_white_attackers, and get_black_attackers are 
// the only functions called from other files. the first two are used to find legal moves in a position, the second two are used to get a mask of all squares that are attacked 
// by the opposing side to determine if the king is in check or if castling is legal (to be optimized)
// the search function first allocates an arrays of Move structs an the stack then passes a pointer into those interfacing functions. Each piece for that side is then 
// called to fill the array with its moves, passing in a movptr to each function that is iterated as the array is filled to keep track of the next empty spot

// GET [PIECE] ATTACKS
// used for pieces that have a complicated function for getting attacking squares (bishops, rooks, and queens basically, but queens can steal rook and bishop implementation)
unsigned long long get_white_rook_attacks(const unsigned long long* board, unsigned long long rook){
    // uses a lookup table of legal sliding rook moves to get legal moves in O(1) time
    // the bit postiions of the pieces in the way are read as an 8 bit integer (8 squares that a rook can move to/from horizontally and vertically)
    // anti_diag mask is used to transpose the board state to calcualte vertical moves as if they were horiztonal
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
    // see get_white_rook_attacks
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

unsigned long long get_white_bishop_attacks(const unsigned long long* board, unsigned long long bishop){
    // in the future i want to implement a similar lookup table strategy as the rook moves
    // for now, uses many bitwise operations to calculate bishop moves in O(1) time
    
    unsigned long long up, down, mask, temp, first_pc, spots;
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    const unsigned long long pcs = whites | blacks;
    int pos = __builtin_ctzll(bishop);
    up = DIAGS_UP[(pos&7) + (pos>>3)]; // lookup tables to get up and right diagnoal moves (assuming empty board for now)
    down = DIAGS_DOWN[7 + (pos>>3) - (pos&7)]; // same for down and right
    
    const unsigned long long sqs_ahead = ~((bishop - 1) | bishop); // all squares ahead of the bishop (as in all squares you would see before the bishop if reading the board like a book)
    const unsigned long long pcs_ahead = pcs & sqs_ahead; // all pcs ahead of the bishop
    const unsigned long long pcs_behind = pcs & (bishop - 1); // all pcs behind the bishop
    
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
    spots |= ((first_pc - 1) & sqs_ahead & down) | (first_pc & blacks);
    
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
    // see get_white_bishop_attacks
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
    spots |= ((first_pc - 1) & sqs_ahead & down) | (first_pc & whites);
    
    // down and right
    temp = pcs_behind & down;
    mask = (!temp) - 1;
    first_pc = (A8 >> __builtin_clzll(temp)) & mask;
    spots |= (bishop - 1) & ~((first_pc - 1) | first_pc) & down;
    spots |= first_pc & whites;
    spots |= down & (bishop - 1) & ~mask;
    return spots;
}

// GET ATTACKS
// Used to determine if king is in check or if castling is legal

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

// GET LEGAL [PIECE] MOVES
// similar implementation for each, iterates over all legal moves, seperating capturing moves, promoting moves, empty moves, and capturing and promoting moves
// fills mov array by calling create_move functions while incrementing movptr to the next empty spot
void get_white_knight_moves(Move** movptr,const unsigned long long* board){

    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];

    unsigned long long knights = board[WHITE_KNIGHT];
    unsigned long long knight, taking_move, taken_piece;

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; knights; knights &= (knights - 1)){
        knight = knights & -knights;
        unsigned long long moves = KNIGHT_MOVES[__builtin_ctzll(knight)] & ~whites;
        for (unsigned long long clear_moves = moves & ~blacks; clear_moves; clear_moves &= (clear_moves - 1)){
            unsigned long long clear_move = (clear_moves & -clear_moves) | knight;
            *movptr = create_move1(*movptr,WHITE_KNIGHT,clear_move,info_xor); 
        }
        for (unsigned long long taking_moves = moves & blacks; taking_moves; taking_moves &= (taking_moves - 1)){
            taking_move = (taking_moves & -taking_moves) | knight;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if ((taken_piece = (taking_move & board[pc]))){
                    *movptr = create_move2(*movptr,WHITE_KNIGHT,taking_move,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }        
        }
    }
}

void get_black_knight_moves(Move** movptr, const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long knights = board[BLACK_KNIGHT];
    unsigned long long knight, taking_move, taken_piece;

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; knights; knights &= (knights - 1)){
        knight = knights & -knights;
        unsigned long long moves = KNIGHT_MOVES[__builtin_ctzll(knight)] & ~blacks;
        for (unsigned long long clear_moves = moves & ~whites; clear_moves; clear_moves &= (clear_moves - 1)){
            unsigned long long clear_move = (clear_moves & (~clear_moves + 1)) | knight;
            *movptr = create_move1(*movptr,BLACK_KNIGHT,clear_move,info_xor); 
        }
        for (unsigned long long taking_moves = moves & whites; taking_moves; taking_moves &= (taking_moves - 1)){
            taking_move = (taking_moves & (~taking_moves + 1)) | knight;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if ((taken_piece = (taking_move & board[pc]))){
                    *movptr = create_move2(*movptr,BLACK_KNIGHT,taking_move,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }        
        }
    }
}

void get_white_rook_moves(Move** movptr, const unsigned long long* board){
    unsigned long long rooks = board[WHITE_ROOK];

    unsigned long long rook, spot, taken_piece; 

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
            *movptr = create_move1(*movptr,WHITE_ROOK,spot,castling_rights_info_xor);
        }
        
        // taking moves
        for(;taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | rook;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,WHITE_ROOK,spot,pc,taken_piece,castling_rights_info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
}

void get_black_rook_moves(Move** movptr, const unsigned long long* board){
    unsigned long long rooks = board[BLACK_ROOK];

    unsigned long long rook, spot, taken_piece; 

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for(;rooks; rooks &= (rooks - 1)){
        rook = rooks & -rooks;
        unsigned long long moves = get_black_rook_attacks(board,rook);
        unsigned long long castling_rights_info_xor = info_xor | (board[INFO] & rook & (BLACK_KINGSIDE_RIGHT | BLACK_QUEENSIDE_RIGHT)); 
        // empty moves
        for(unsigned long long spots = moves & ~board[WHITE_PCS]; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | rook;
            *movptr = create_move1(*movptr,BLACK_ROOK,spot,castling_rights_info_xor);
        }
        
        // taking moves
        for(unsigned long long taking_spots = moves & board[WHITE_PCS]; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | rook;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,BLACK_ROOK,spot,pc,taken_piece,castling_rights_info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
    }

void get_white_bishop_moves(Move** movptr, const unsigned long long* board){
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long bishops = board[WHITE_BISHOP];
    unsigned long long bishop, spot, taken_piece;
    
    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; bishops; bishops &= (bishops - 1)){
        bishop = bishops & -bishops;
        const unsigned long long moves = get_white_bishop_attacks(board,bishop);
        // empty moves
        for(unsigned long long spots = moves & ~blacks; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | bishop;
            *movptr = create_move1(*movptr,WHITE_BISHOP,spot,info_xor);
        }
        
        // taking moves
        for(unsigned long long taking_spots = moves & blacks; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | bishop;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,WHITE_BISHOP,spot,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
}

void get_black_bishop_moves(Move** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    unsigned long long bishops = board[BLACK_BISHOP];

    unsigned long long bishop, spot, taken_piece;
    

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; bishops; bishops &= (bishops - 1)){
        bishop = bishops & -bishops;
        const unsigned long long moves = get_black_bishop_attacks(board,bishop);
        
        for(unsigned long long spots = moves & ~whites; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | bishop;
            *movptr = create_move1(*movptr,BLACK_BISHOP,spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & whites; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | bishop;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,BLACK_BISHOP,spot,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
}

void get_white_pawn_moves(Move** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long pawns = board[WHITE_PAWN];
    unsigned long long pcs = whites | blacks;
    unsigned long long spot, taken_piece, promotion_sq;

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    // step forward one
    for (unsigned long long one_step = (pawns << 8) & ~pcs; one_step; one_step &= (one_step - 1)){
        spot = (one_step & (~one_step + 1));
        spot |= spot >> 8;
        if ((promotion_sq = (spot & RANK_8))){
            *movptr = create_move2(*movptr,WHITE_PAWN,spot,WHITE_QUEEN,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,WHITE_PAWN,spot,WHITE_KNIGHT,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,WHITE_PAWN,spot,WHITE_ROOK,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,WHITE_PAWN,spot,WHITE_BISHOP,promotion_sq,info_xor,PROMOTE);
        } else {
            *movptr = create_move1(*movptr,WHITE_PAWN,spot,info_xor);
        }
    }
    // step forward two
    for (unsigned long long two_step = ((pawns & RANK_2) << 16) & ~((pcs) | (pcs << 8)); two_step; two_step &= (two_step - 1)){
        spot = (two_step & (~two_step + 1));
        unsigned long long info_xor_with_enpassent = (spot >> 8) | info_xor; // creating en passent
        spot |= spot >> 16;
        *movptr = create_move1(*movptr,WHITE_PAWN,spot,info_xor_with_enpassent);
    }

    // taking right
    for (unsigned long long take_right = (pawns << 7) & blacks & ~FILE_A; take_right; take_right &= (take_right - 1)){
        spot = (take_right & (~take_right + 1));
        spot |= spot >> 7;
        for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
            if ((taken_piece = (spot & board[pc]))){
                if ((promotion_sq = (spot & RANK_8))){
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_QUEEN,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_KNIGHT,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_ROOK,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_BISHOP,promotion_sq,info_xor);
                } else {
                    *movptr = create_move2(*movptr,WHITE_PAWN,spot,pc,taken_piece,info_xor,CAPTURE);
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
            if ((taken_piece = (spot & board[pc]))){
                if ((promotion_sq = (spot & RANK_8))){
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_QUEEN,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_KNIGHT,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_ROOK,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,WHITE_PAWN,spot,pc,taken_piece,WHITE_BISHOP,promotion_sq,info_xor);
                } else {
                    *movptr = create_move2(*movptr,WHITE_PAWN,spot,pc,taken_piece,info_xor,CAPTURE);
                }
                break;
            }
        }
    }
    // taking en passent 
    unsigned long long en_passent_take_left = ((pawns << 9) & board[INFO] & ~RANK_1);
    if (en_passent_take_left){
        *movptr = create_move2(*movptr,WHITE_PAWN,en_passent_take_left | (en_passent_take_left >> 9),
                                    BLACK_PAWN,en_passent_take_left >> 8,info_xor,CAPTURE);
    }
    
    unsigned long long en_passent_take_right = ((pawns << 7) & board[INFO] & ~RANK_1);
    if (en_passent_take_right){
        *movptr = create_move2(*movptr,WHITE_PAWN,en_passent_take_right | (en_passent_take_right >> 7),
                                    BLACK_PAWN,en_passent_take_right >> 8,info_xor,CAPTURE);
    }

}

void get_black_pawn_moves(Move** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long pawns = board[BLACK_PAWN];
    unsigned long long pcs = whites | blacks;
    unsigned long long spot, taken_piece, promotion_sq;

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (unsigned long long one_step = (pawns >> 8) & ~pcs; one_step; one_step &= (one_step - 1)){
        spot = (one_step & (~one_step + 1));
        spot |= spot << 8;
        if ((promotion_sq = (spot & RANK_1))){
            *movptr = create_move2(*movptr,BLACK_PAWN,spot,BLACK_QUEEN,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,BLACK_PAWN,spot,BLACK_KNIGHT,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,BLACK_PAWN,spot,BLACK_BISHOP,promotion_sq,info_xor,PROMOTE);
            *movptr = create_move2(*movptr,BLACK_PAWN,spot,BLACK_ROOK,promotion_sq,info_xor,PROMOTE);
        } else {
            *movptr = create_move1(*movptr,WHITE_PAWN,spot,info_xor);
        }
    }
    for (unsigned long long two_step = ((pawns & RANK_7) >> 16) & ~((pcs) | (pcs >> 8)); two_step; two_step &= (two_step - 1)){
        spot = (two_step & (~two_step + 1));
        unsigned long long info_xor_with_enpassent = (spot << 8) | info_xor;
        spot |= spot << 16;
        *movptr = create_move1(*movptr,BLACK_PAWN,spot,info_xor_with_enpassent);
    }
    for (unsigned long long take_right = (pawns >> 9) & whites & ~FILE_A; take_right; take_right &= (take_right - 1)){
        spot = (take_right & (~take_right + 1));
        spot |= spot << 9;
        for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
            if ((taken_piece = (spot & board[pc]))){
                if ((promotion_sq = (spot & RANK_1))){
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_QUEEN,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_KNIGHT,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_BISHOP,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_ROOK,promotion_sq,info_xor);
                } else {
                    *movptr = create_move2(*movptr,BLACK_PAWN,spot,pc,taken_piece,info_xor,CAPTURE);
                }
                break;
            }
        }
    }
    for (unsigned long long take_left = (pawns >> 7) & whites & ~FILE_H; take_left; take_left &= (take_left - 1)){
        spot = (take_left & (~take_left + 1));
        spot |= spot << 7;
        for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
            if ((taken_piece = (spot & board[pc]))){
                if ((promotion_sq = (spot & RANK_1))){
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_QUEEN,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_KNIGHT,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_BISHOP,promotion_sq,info_xor);
                    *movptr = create_move3(*movptr,BLACK_PAWN,spot,pc,taken_piece,BLACK_ROOK,promotion_sq,info_xor);
                } else {
                    *movptr = create_move2(*movptr,BLACK_PAWN,spot,pc,taken_piece,info_xor,CAPTURE);
                }
                break;
            }
        }
    }

    // taking en passent 
    unsigned long long en_passent_take_left = ((pawns >> 7) & board[INFO] & ~RANK_1);
    if (en_passent_take_left){
        *movptr = create_move2(*movptr,BLACK_PAWN,en_passent_take_left | (en_passent_take_left << 7),
                                   WHITE_PAWN,en_passent_take_left << 8,info_xor,CAPTURE);
    }
    
    unsigned long long en_passent_take_right = ((pawns >> 9) & board[INFO] & ~RANK_1);
    if (en_passent_take_right){
        *movptr = create_move2(*movptr,BLACK_PAWN,en_passent_take_right | (en_passent_take_right << 9),
                                   WHITE_PAWN,en_passent_take_right << 8,info_xor,CAPTURE);
    }
}

void get_white_king_moves(Move** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long king = board[WHITE_KING];
    unsigned long long moves = KING_MOVES[__builtin_ctzll(king)] & ~whites;
    unsigned long long move, taken_piece;

    const unsigned long long info_xor = TURN_BIT | ((WHITE_KINGSIDE_RIGHT | WHITE_QUEENSIDE_RIGHT) & board[INFO]);

    for(unsigned long long clear_moves = moves & ~blacks; clear_moves; clear_moves &= (clear_moves - 1)){
        move = (clear_moves & (-clear_moves)) | king;
        *movptr = create_move1(*movptr,WHITE_KING, move, info_xor);
    }
    
    for(unsigned long long taking_moves = moves & blacks; taking_moves; taking_moves &= (taking_moves - 1)){
        move = (taking_moves & (-taking_moves)) | king;
        for (int pc = BLACK_PAWN; pc < BLACK_KING; pc++){
            if ((taken_piece = board[pc] & move)){
                *movptr = create_move2(*movptr,WHITE_KING,move,pc,taken_piece,info_xor,CAPTURE);
            }
        }        
    } 
    unsigned long long attacks = 0;
    if ((board[INFO] & WHITE_KINGSIDE_RIGHT) && 
    ((WHITE_KINGSIDE_SPACE & (whites | blacks)) == 0) &&
    (board[WHITE_ROOK] & FILE_H & RANK_1)){
        attacks = get_black_attackers(board);
        if ((attacks & WHITE_KINGSIDE_ATTACKED) == 0){
            *movptr = create_move2(*movptr,WHITE_KING,0b1010ULL,WHITE_ROOK,0b0101ULL,info_xor,CAPTURE);
        }
    }
    if ((board[INFO] & WHITE_QUEENSIDE_RIGHT) && 
        ((WHITE_QUEENSIDE_SPACE & (whites | blacks)) == 0) &&
        ((board[WHITE_ROOK] & FILE_A & RANK_1))){
            if (attacks == 0) attacks = get_black_attackers(board);
            
            if ((attacks & WHITE_QUEENSIDE_ATTACKED) == 0){
                *movptr = create_move2(*movptr,WHITE_KING,0b101000ULL,WHITE_ROOK,0b10010000ULL,info_xor,CAPTURE);
            }
    }

}

void get_black_king_moves(Move** movptr,const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long king = board[BLACK_KING];
    unsigned long long moves = KING_MOVES[__builtin_ctzll(king)] & ~blacks;
    unsigned long long move, taken_piece;


    const unsigned long long info_xor = TURN_BIT | ((BLACK_KINGSIDE_RIGHT | BLACK_QUEENSIDE_RIGHT) & board[INFO]);

    for(unsigned long long clear_moves = moves & ~whites; clear_moves; clear_moves &= clear_moves - 1){
        move = (clear_moves & (-clear_moves)) | king;
        *movptr = create_move1(*movptr,BLACK_KING,move,info_xor);
    }

    for(unsigned long long taking_moves = moves & whites; taking_moves; taking_moves &= taking_moves - 1){
        move = (taking_moves & (-taking_moves)) | king;
        for (int pc = WHITE_PAWN; pc < WHITE_KING; pc++){
            if ((taken_piece = board[pc] & move)){
                *movptr = create_move2(*movptr,BLACK_KING,move,pc,taken_piece,info_xor,CAPTURE);
            }
        }        
    }
    unsigned long long attacks;
    if ((board[INFO] & BLACK_KINGSIDE_RIGHT) && 
        ((BLACK_KINGSIDE_SPACE & (whites | blacks)) == 0) &&
        (board[BLACK_ROOK] & FILE_H & RANK_8)){
            attacks = get_white_attackers(board);
            if ((attacks & BLACK_KINGSIDE_ATTACKED) == 0){
                *movptr = create_move2(*movptr,BLACK_KING,0b1010ULL << 56,BLACK_ROOK,0b0101ULL << 56,info_xor,CAPTURE);
            }
    }

    if ((board[INFO] & BLACK_QUEENSIDE_RIGHT) && 
        ((BLACK_QUEENSIDE_SPACE & (whites | blacks)) == 0) &&
        (board[BLACK_ROOK] & FILE_A & RANK_8)){
            if (attacks == 0) attacks = get_white_attackers(board);
            
            if ((attacks & BLACK_QUEENSIDE_ATTACKED) == 0){
                *movptr = create_move2(*movptr,BLACK_KING,0b101000ULL << 56,BLACK_ROOK,0b10010000ULL << 56,info_xor,CAPTURE);
            }
    }
}

void get_white_queen_moves(Move** movptr, const unsigned long long* board){
    const unsigned long long blacks = board[BLACK_PCS];
    unsigned long long queens = board[WHITE_QUEEN];
    unsigned long long queen, spot, taken_piece;
    
    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; queens; queens &= (queens - 1)){
        queen = queens & -queens;
        const unsigned long long moves = get_white_bishop_attacks(board,queen) | get_white_rook_attacks(board,queen);
        
        for(unsigned long long spots = moves & ~blacks; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | queen;
            *movptr = create_move1(*movptr,WHITE_QUEEN,spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & blacks; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | queen;
            for (int pc = BLACK_PAWN; pc <= BLACK_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,WHITE_QUEEN,spot,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
}

void get_black_queen_moves(Move** movptr, const unsigned long long* board){
    const unsigned long long whites = board[WHITE_PCS];
    unsigned long long queens = board[BLACK_QUEEN];
    unsigned long long queen, spot, taken_piece;

    unsigned long long info_xor = TURN_BIT | (board[INFO] & ~RANK_1 & ~RANK_8);

    for (; queens; queens &= (queens - 1)){
        queen = queens & -queens;
        const unsigned long long moves = get_black_bishop_attacks(board,queen) | get_black_rook_attacks(board,queen);
        
        for(unsigned long long spots = moves & ~whites; spots; spots &= (spots - 1)){
            spot = (spots & (~spots + 1)) | queen;
            *movptr = create_move1(*movptr,BLACK_QUEEN,spot,info_xor);
        }
        // taking moves
        for(unsigned long long taking_spots = moves & whites; taking_spots; taking_spots &= (taking_spots - 1)){
            spot = (taking_spots & (~taking_spots + 1)) | queen;
            for (int pc = WHITE_PAWN; pc <= WHITE_QUEEN; pc++){
                if ((taken_piece = (spot & board[pc]))){
                    *movptr = create_move2(*movptr,BLACK_QUEEN,spot,pc,taken_piece,info_xor,CAPTURE);
                    break;
                }
            }
        }
    }
}

// GET LEGAL MOVES
// used to get all legal moves given a board position

// passed into q_sort to apply move sorting heuristics to improve alpha-beta pruning efficiency
int compare_moves(const void* a, const void* b){
    const Move* movA = (const Move*)a;
    const Move* movB = (const Move*)b;
    if (movA->type != movB->type){
        return movB->type - movA->type;
    }
    switch (movA->type){
        case EMPTY:
            return movB->pc2 - movA->pc2;
        case CAPTURE:
            return ((movB->pc2 - movA->pc2) * 16) + (movA->pc1 - movB->pc1);
        case PROMOTE:
            return movB->pc2 - movA->pc2;
        case CAPTURE_PROMOTE:
            return ((movB->pc3 - movA->pc3) * 256) + ((movB->pc2 - movA->pc2) * 16) + (movA->pc1 - movB->pc1);
        default:
            return 0;
    }
}

// white side interfacing function
void get_white_moves(Move* movs,const unsigned long long* board){
    Move* movptr = movs;
    get_white_queen_moves(&movptr,board);
    get_white_rook_moves(&movptr,board);
    get_white_bishop_moves(&movptr,board);
    get_white_knight_moves(&movptr,board);
    get_white_pawn_moves(&movptr,board);
    get_white_king_moves(&movptr,board);
    movptr->type = BOOK_END;

    // apply move ordering
    qsort(movs, movptr-movs, sizeof(Move), compare_moves);
}

// black side interfacing function
void get_black_moves(Move* movs, const unsigned long long* board){
    Move* movptr = movs;
    get_black_queen_moves(&movptr,board);
    get_black_rook_moves(&movptr,board);
    get_black_bishop_moves(&movptr,board);
    get_black_knight_moves(&movptr,board);
    get_black_pawn_moves(&movptr,board);
    get_black_king_moves(&movptr,board);
    movptr->type = BOOK_END;
    
    // apply move ordering
    qsort(movs, movptr-movs, sizeof(Move), compare_moves);
}