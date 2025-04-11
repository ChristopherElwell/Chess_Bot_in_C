// Author: Christopher Elwell
// My (unfinished) implementation of a chess engine. All code by me. 
// 
// Board representation is with 64 bit integers, one for each piece. Each square maps to one bit within the integer: 1 if a piece is there, 0 if not.
// Each piece is represented this way, allowing for fast move generation and evaluation. Definitely an inefficient storage method in terms of memory usage, 
// but there is only one board array at any given time, except for in the transposition table, which uses a different board state encoding that is memory efficient.
// 
// Uses alpha-beta pruning, described by wikipedia:
//      "It stops evaluating a move when at least one possibility has been found that proves the move to be worse than a previously examined move. 
//      Such moves need not be evaluated further."

#include "constants.h"
#include "get_moves.h"
#include "search.h"
#include "helpers.h"
#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>

# define SEARCH_TIME 500 // 500 ms per move

// main.c acts as a interface between the controller (written in Python) and the engine itself, mostly boilerplate stuff here

char* get_bot_move(char* FEN){
    clock_t start = clock();
    printf("%s\n",FEN);
    uint64_t* board = from_FEN(FEN);
    searchResult* bot_move;

    // iterative deepening
    int i = 1;
    printf("Depth: ");
    while((double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC < SEARCH_TIME){
        printf("%d, ",i);
        bot_move = search(board,i,INT16_MIN,INT16_MAX);
        i++;
    }
    
    // print information, return best move to controller
    printf("Eval: %d\n",bot_move->best_eval);
    print_principal_variation(bot_move,board);
    char* move = move_to_uci(&(bot_move->best_move),board);
    free_search_result(bot_move);
    return move;
}

char* debug_get_bot_move(int depth,char* FEN){
    printf("%s\n",FEN);
    uint64_t* board = from_FEN(FEN);
    uint64_t* copy = from_FEN(FEN);

    searchResult* bot_move = search(board,depth,INT16_MIN,INT16_MAX);
    for (int pc = WHITE_PAWN; pc <= INFO; pc++){
        if (copy[pc] != board[pc]){
            printf("%d DIFFERENT\n",pc);
        } else {
            
            printf("%d SAME\n",pc);
        }
    }
    // print_move(&(bot_move->best_move));
    print_principal_variation(bot_move,board);
    return move_to_uci(&(bot_move->best_move),board);
}

int testing(char* fen,bool print_moves){
    return 0;
}

void receiver() {
    char buffer[256];
    char FEN[256];
    // constantly searches for message from controller
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0; 
        if (strncmp(buffer, "GET ", 4) == 0) {
            char message[256] = "";
            snprintf(message, sizeof(message), "%s", buffer + 4);

            // call testing func
            if (strncmp(message, "TEST ", 5) == 0) {
                snprintf(FEN, sizeof(FEN), "%s", message + 5);
                testing(FEN, false);
                fflush(stderr);
            } 
            // call bot to return best move
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
    // return 0;
}

