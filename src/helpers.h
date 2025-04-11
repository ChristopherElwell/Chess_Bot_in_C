#pragma once
#include "constants.h"
#include "get_moves.h"
#include "search.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void free_search_result (searchResult *sr);
void free_board (uint64_t *board);
uint64_t sq_from_name (char file, char rank);
void print_bit_board (const uint64_t b);
void print_board (uint64_t *const board);
void print_move (Move *m);
void print_move_short (Move *m);
void apply_move (Move *m, uint64_t *board);
Move *create_move1 (Move *m, int pc, uint64_t mov, uint64_t info_in);
Move *create_move2 (Move *m, int pc1, uint64_t mov1, int pc2, uint64_t mov2, uint64_t info_in, enum movType type);
Move *create_move3 (Move *m, int pc1, uint64_t mov1, int pc2, uint64_t mov2, int pc3, uint64_t mov3, uint64_t info_in);
void prep_board (uint64_t *board);
Move copy_move (const Move *original);
uint64_t *from_FEN (const char *p);
char *move_to_uci (Move *mov, uint64_t *board);
void print_principal_variation (searchResult *sr, uint64_t *board);
void read_pos_csv(const char* filename, char** FENs, int num_rows);
