#pragma once
#include <stdint.h>
#include "constants.h"

typedef enum movType{
  EMPTY,
  CAPTURE,
  PROMOTE,
  CAPTURE_PROMOTE,
  BOOK_END
} movType;

typedef struct Move {
    movType type;
    uint64_t mov1, mov2, mov3;
    int pc1, pc2, pc3;
    uint64_t info;
} Move;
  
unsigned long long get_white_attackers(const unsigned long long* board);
unsigned long long get_black_attackers(const unsigned long long* board);
void get_black_moves(Move* movs, const unsigned long long* board);
void get_white_moves(Move* movs,const unsigned long long* board);