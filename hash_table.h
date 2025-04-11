# pragma once
#include "constants.h"
#include "get_moves.h"
#include <stdint.h>

#define PRIME UINT64_C(0x9E3779B97F4A7C55)
#define MAX_LINKED_LIST 4 

typedef struct Node {
    uint8_t search_info;
    uint64_t move_code;
    struct Node* next;
    uint64_t hash;
} Node;

extern Node** TransTable;

uint64_t get_hash(uint64_t* board);
void initilize_trans_table();
void free_trans_table();
void add_item(Move* in_m, int type, uint64_t* board);
void hash_testing();
Move decrypt_move(uint64_t code);
uint64_t encrypt_move(Move* m);