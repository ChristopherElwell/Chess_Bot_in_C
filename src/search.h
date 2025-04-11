#pragma once
#include "constants.h"
#include "stdbool.h"
#include "get_moves.h"

# define CHECKMATE_EVAL 30000
# define EARLY_CHECKMATE_INCENTIVE 2000

typedef struct SearchResult{
    Move best_move;
    int16_t best_eval;
    struct SearchResult* best_result;
  } searchResult;

searchResult *search(long long unsigned int *board, int iter, int16_t alpha, int16_t beta);
