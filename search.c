#include "search.h"
#include "constants.h"
#include "eval.h"
#include "helpers.h"
#include "get_moves.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

// main serach function
searchResult* search(unsigned long long* board, int iter, int16_t alpha, int16_t beta){    
    searchResult* this_result = malloc(sizeof(searchResult));
    this_result->best_result = NULL;
    this_result->best_move.type = BOOK_END;
    
    // if end of iteration, return evaluation of board
    if (!iter){
        this_result->best_eval = evaluate(board);
        return this_result;
    }
    
    // initialize move array large enough for most possible moves in a chess position (219) with head room (240) to account for varying piece promotions
    Move movs[MOVES_ARRAY_LENGTH];
    if (board[INFO] & TURN_BIT){ // white
        this_result->best_eval = INT16_MIN;
        get_white_moves(movs,board);
        Move* best_move_ptr = NULL;
        
        for (Move* movptr = &(movs[0]); movptr -> type != BOOK_END; movptr++){
            apply_move(movptr,board);
           
            assert((board[INFO] & TURN_BIT) == 0);
            
            // check if move leaves white king in check
            if (board[WHITE_KING] & get_black_attackers(board)){
                apply_move(movptr,board);
                continue;
            }
            
            searchResult* child_result = search(board, iter - 1, alpha, beta);
            apply_move(movptr,board);


            if (child_result->best_eval > this_result->best_eval){
                free_search_result(this_result->best_result); // free old best move
                this_result->best_result = child_result; // set new best move to this one
                this_result->best_eval = child_result->best_eval; // set new best eval to this one
                best_move_ptr = movptr; // set new best move ptr to this one
                alpha = max(child_result->best_eval,alpha); // update alpha
            }             

            // alpha-beta pruning
            if (this_result->best_eval >= beta){
                break;
                if (this_result -> best_result != child_result){
                    free_search_result(child_result);
                }
            } 
            // if this move was not chosen, free memory
            if (this_result -> best_result != child_result){
                free_search_result(child_result);
            }
        }
        // copy best move to search return
        if (best_move_ptr != NULL){
            this_result->best_move = copy_move(best_move_ptr);
        } else { // if no valid move found we have either a checkmate or a stalemate
            // test if king is in check to determine checkmate vs stalemate, apply checkmate eval function to incentivise checkmates as soon as possible (prevents playing wiht the opponent forever)
            this_result->best_eval = (board[WHITE_KING] & get_black_attackers(board)) ? -(CHECKMATE_EVAL - EARLY_CHECKMATE_INCENTIVE / (iter + 1)) : 0;
        }
    } else { // black
        this_result->best_eval = INT16_MAX;
        get_black_moves(movs, board);
        Move* best_move_ptr = NULL;

        for (Move* movptr = &(movs[0]); movptr -> type != BOOK_END; movptr++){
            apply_move(movptr,board);
            
            assert(board[INFO] & TURN_BIT);
            
            // check if move leaaves black king in check
            if (board[BLACK_KING] & get_white_attackers(board)){
                apply_move(movptr,board);
                continue;
            }
            searchResult* child_result = search(board, iter - 1, alpha, beta);
            apply_move(movptr,board);
            
            if (child_result->best_eval < this_result->best_eval){ // same as for white
                free_search_result(this_result->best_result);
                this_result->best_result = child_result;
                this_result->best_eval = child_result->best_eval;
                best_move_ptr = movptr;
                beta = min(child_result->best_eval,beta);  
            } 
            if (child_result->best_eval <= alpha){
                break;
                if (this_result -> best_result != child_result){
                    free_search_result(child_result);
                }
            }
            if (this_result -> best_result != child_result){
                free_search_result(child_result);
            }
        }
        // same as for white
        if (best_move_ptr != NULL){
            this_result->best_move = copy_move(best_move_ptr);
        } else {
            this_result->best_eval = (board[BLACK_KING] & get_white_attackers(board)) ? (CHECKMATE_EVAL - EARLY_CHECKMATE_INCENTIVE / (iter + 1)) : 0;
        }
    }
    return this_result;
}