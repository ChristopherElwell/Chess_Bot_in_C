#include "constants.h"
#include "func_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

# define MAX_ITER 4

void print_bit_board(const unsigned long long b);
int evaluate(const unsigned long long* board);
void apply_move(Move* m, unsigned long long* board);
void print_board(unsigned long long* const board);
void print_move(Move* m);
Move** get_white_moves(const unsigned long long* board);
Move** get_black_moves(const unsigned long long* board);
unsigned long long get_white_attackers(const unsigned long long* board);
unsigned long long get_black_attackers(const unsigned long long* board);
int search_white_move(unsigned long long* board, int iter);
int search_black_move(unsigned long long* board, int iter);
void free_move(Move* m);
Move* copy_move(const Move* original);

int max(int a, int b){
    if (a > b){
        return a;
    }
    return b;
}

int min(int a, int b){
    if (a < b){
        return a;
    }
    return b;
}

bool compare_boards(unsigned long long* board1,unsigned long long* board2){
    for (int i = 0; i < BOARD_ARRAY_SIZE; i++){
        if (board1[i] != board2[i]) return false;
    }
    return true;
}

int search_white_move(unsigned long long* board, int iter){
    unsigned long long original[BOARD_ARRAY_SIZE];
    for (int i = 0; i < BOARD_ARRAY_SIZE; i++){
        original[i] = board[i];
    }
    if (iter <= 0) return evaluate(board);

    int legal_moves = 0;

    Move* best_move = NULL;

    int best_eval = INT_MIN;
    int eval;
    assert(board[INFO] & TURN_BIT);
    
    Move** movs = get_white_moves(board);
    for (Move** movptr = movs; *movptr != NULL; movptr++){
        apply_move(*movptr,board);
        
        assert((board[INFO] & TURN_BIT) == 0);

        if (board[WHITE_KING] & get_black_attackers(board)){
            apply_move(*movptr,board);
            continue;
        }

        legal_moves++;

        eval = search_black_move(board,iter-1);
        best_eval = max(best_eval,eval);

        apply_move(*movptr,board);

        if (eval > best_eval){
            best_move = copy_move(*movptr);
            best_eval = eval;
        }
        
        free_move(*movptr);
        
        assert(compare_boards(board,original));

    }
    
    if (legal_moves == 0){
        if (board[WHITE_KING] & get_black_attackers(board)){
            return INT_MIN;
        } else {
            return 0;
        }
    }

    return best_eval;
}

int search_black_move(unsigned long long* board, int iter){
    unsigned long long original[BOARD_ARRAY_SIZE];
    for (int i = 0; i < BOARD_ARRAY_SIZE; i++){
        original[i] = board[i];
    }
    if (iter <= 0) return evaluate(board);

    int legal_moves = 0;

    int best_eval = INT_MAX;
    int eval;
    Move* best_move = NULL;
    assert((board[INFO] & TURN_BIT) == 0);
    
    Move** movs = get_black_moves(board);
    for (Move** movptr = movs; *movptr != NULL; movptr++){
        apply_move(*movptr,board);

        assert(board[INFO] & TURN_BIT);

        if (board[BLACK_KING] & get_white_attackers(board)){
            apply_move(*movptr,board);
            continue;
        } 

        legal_moves++;

        eval = search_white_move(board,iter - 1);
        
        apply_move(*movptr,board);
        
        if (eval < best_eval){
            best_move = copy_move(*movptr);
            best_eval = eval;
        }
        
        free_move(*movptr);
        assert(compare_boards(board,original));

    }

    if (legal_moves == 0){
        if (board[BLACK_KING] & get_white_attackers(board)){
            return INT_MAX;
        } else {
            return 0;
        }
    }

    return best_eval;
}

Move* root_search(unsigned long long* board,int max_iter){    
    unsigned long long original[BOARD_ARRAY_SIZE];
    for (int i = 0; i < BOARD_ARRAY_SIZE; i++){
        original[i] = board[i];
    }
    int eval;
    Move* best_move = NULL;
    int best_eval;
    if (board[INFO] & TURN_BIT){
        best_eval = INT_MIN;
        Move** movs = get_white_moves(board);
        for (Move** movptr = movs; *movptr != NULL; movptr++){
            apply_move(*movptr,board);
            assert((board[INFO] & TURN_BIT) == 0);
            
            if (board[WHITE_KING] & get_black_attackers(board)){
                apply_move(*movptr,board);
                continue;
            } 
            
            eval = search_black_move(board,max_iter);
            apply_move(*movptr,board);
            
            if (eval > best_eval){
                best_eval = eval;
                best_move = copy_move(*movptr);
            } 
            
            // printf("EVAL: %d\n",eval);
            // print_move(*movptr);
            
            free_move(*movptr);
            assert(compare_boards(board,original));
        }
        free(movs);
    } else {
        best_eval = INT_MAX;
        Move** movs = get_black_moves(board);
        for (Move** movptr = movs; *movptr != NULL; movptr++){
            apply_move(*movptr,board);
            assert((board[INFO] & TURN_BIT) != 0);

            if (board[BLACK_KING] & get_white_attackers(board)) {
                apply_move(*movptr,board);
                free_move(*movptr);
                continue;
            }
            
            eval = search_white_move(board,max_iter);
            apply_move(*movptr,board);
            
            if (eval > best_eval){
                best_eval = eval;
                best_move = copy_move(*movptr);
            } 
            free_move(*movptr);
            
            assert(compare_boards(board,original));
        }
        free(movs);
    }
    return best_move;
}