#include "hash_table.h"
#include "constants.h"
#include "get_moves.h"
#include "helpers.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// CUSTOM HASH TABLE IMPLEMENTATION
// uses 64 bit hashses, first indexed in the array with the least significant 16 bits, then compared with the full 64 along a linked list with head in the array
// stores previously found best move for a given lookup position. 
// called a transposition table because it is primarily used to skip searching when the same board positions shows up again from a different series of moves, this is called a transposition

#define pc1_code_shift    2
#define mov1_1_code_shift 6
#define mov1_2_code_shift 12
#define pc2_code_shift    18
#define mov2_1_code_shift 22
#define mov2_2_code_shift 28
#define pc3_code_shift    34
#define mov3_code_shift   38
#define Kcstle_code_shift 44
#define Qcstle_code_shift 45
#define kcstle_code_shift 46
#define qcstle_code_shift 47
#define turn_code_shift   48
#define enpas1_code_shift 49
#define enpas2_code_shift 55

// type   0-1
// pc1    2-5
// mov1,1 6-11
// mov1,2 12-17
// pc2    18-21
// mov2,1 22-27
// mov2,2 28-33
// pc3    34-37
// mov3   38-43
// Kcstle 44
// Qcstle 45
// kcstle 46
// qcstle 47
// turn   48
// enpas1 49-54
// enpas2 55-60

// TRANSTABLE
Node** TransTable;

// HASHING IMPLEMENTATION
uint64_t xorshift64_state = PRIME; // seed for hashing function

// arrays for hashing function
static uint64_t zobrist_pc_keys[12][64];
static uint64_t zobrist_en_pass_keys[8];
static uint64_t zobrist_info_keys[5];

// creates random numbers to fill hashing arrays
uint64_t xorshift64() {
    uint64_t x = xorshift64_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return xorshift64_state = x;
}

// fills hashing arrays
void initialize_zobrist(){
    for (int pc = 0; pc < 12; pc++){
        for (int i = 0; i < 64; i++){
            zobrist_pc_keys[pc][i] = xorshift64();
        }
    }
    for (int i = 0; i < 8; i++){
        zobrist_en_pass_keys[i] = xorshift64();
    }
    for (int i = 0; i < 5; i++){
        zobrist_info_keys[i] = xorshift64();
    }
}

uint64_t get_hash(uint64_t* board){
    uint64_t hash = 0;
    for (int pc = WHITE_PAWN; pc <= INFO; pc++){
        for (uint64_t pieces = board[pc]; pieces; pieces &= (pieces - 1)){
            hash ^= zobrist_pc_keys[pc][__builtin_ctzll(pieces)];
        }
    }
    if (board[INFO] & WHITE_KINGSIDE_RIGHT){
        hash ^= (((board[INFO] & WHITE_KINGSIDE_RIGHT) == 0) - 1) & zobrist_info_keys[0];
    }
    if (board[INFO] & WHITE_QUEENSIDE_RIGHT){
        hash ^= zobrist_info_keys[1];
    }
    if (board[INFO] & BLACK_KINGSIDE_RIGHT){
        hash ^= zobrist_info_keys[2];
    }
    if (board[INFO] & BLACK_QUEENSIDE_RIGHT){
        hash ^= zobrist_info_keys[3];
    }
    if (board[INFO] & TURN_BIT){
        hash ^= zobrist_info_keys[4];
    }
    uint64_t en_pass = board[INFO] & ~RANK_1 & ~RANK_8;
    if (en_pass){
        
    }
    return hash;
}

// HASHTABLE IMPLEMENTATION
void initilize_trans_table(){
    TransTable = malloc(UINT16_MAX * sizeof(Node*)); 
    for (int i = 0; i < UINT16_MAX; i++){
        TransTable[i] = NULL;
    }
}

void free_trans_table(){
    Node* temp;
    for (int i = 0; i < UINT16_MAX; i++){
        Node* next = TransTable[i];
        
        while (next != NULL){
            temp = next->next;
            free(next);
            next = temp;
        }
    }
    free(TransTable);
}

// converts move struct to one 64 bit integer with all information stored, significantly increases the capacity of hashtable given a total memory count
uint64_t encrypt_move(Move* m){
    uint64_t code = 0;
    uint64_t temp;

    // type 
    code |= m->type;
    
    // pc1
    code |= m->pc1 << pc1_code_shift;
    
    // mov1
    code |= (uint64_t)__builtin_ctzll(m->mov1) << mov1_1_code_shift;
    temp = m->mov1 & (m->mov1-1);
    code |= (uint64_t)__builtin_ctzll(temp) << mov1_2_code_shift;
    
    // pc2
    code |= m->pc2 << pc2_code_shift;
    
    // mov2
    code |= (uint64_t)(__builtin_ctzll(m->mov2) & 0x3f) << mov2_1_code_shift;
    temp = m->mov2 & (m->mov2-1);
    code |= (uint64_t)(__builtin_ctzll(temp) & 0x3f) << mov2_2_code_shift;

    // pc3
    code |= ((uint64_t)m->pc3) << pc3_code_shift;
    
    // mov3
    code |= (uint64_t)(__builtin_ctzll(m->mov3) & 0x3f) << mov3_code_shift;
    
    // info
    code |= ((m->info & WHITE_KINGSIDE_RIGHT) << (Kcstle_code_shift - 0)) |
            ((m->info & WHITE_QUEENSIDE_RIGHT) << (Qcstle_code_shift - 7)) |
            ((m->info & BLACK_KINGSIDE_RIGHT) >> (56 - kcstle_code_shift)) |
            ((m->info & BLACK_QUEENSIDE_RIGHT) >> (63 - qcstle_code_shift)) |
            ((m->info & TURN_BIT) << (turn_code_shift - 1));

    temp = m->info & ~RANK_1 & ~RANK_8;
    code |= ((uint64_t)(__builtin_ctzll(temp) & 0x3f) << enpas1_code_shift);
    temp &= (temp - 1);
    code |= ((uint64_t)(__builtin_ctzll(temp) & 0x3f) << enpas2_code_shift);
    return code;
}

// inverse operation of decrypt move, converts 64 bit integer to move struct
Move decrypt_move(uint64_t code){
    Move m;
    
    m.type = (movType)(code & 3);
    m.info = ((code & (1ULL << Kcstle_code_shift)) >> (Kcstle_code_shift - 0)) |
             ((code & (1ULL << Qcstle_code_shift)) >> (Qcstle_code_shift - 7)) |
             ((code & (1ULL << kcstle_code_shift)) << (56 - kcstle_code_shift)) |
             ((code & (1ULL << qcstle_code_shift)) << (63 - qcstle_code_shift)) |
             ((code & (1ULL << turn_code_shift)) >> (turn_code_shift - 1)) |
             ((1ULL << ((code >> enpas1_code_shift) & 0x3f)) & ~1ULL) |
             ((1ULL << ((code >> enpas2_code_shift) & 0x3f)) & ~1ULL);

    m.pc1 = (int)((code >> pc1_code_shift) & 0xf);
    m.mov1 = (1ULL << ((code >> mov1_1_code_shift) & 0x3f)) | (1ULL << ((code >> mov1_2_code_shift) & 0x3f));
    
    if (m.type == EMPTY){
        m.pc2 = 0;
        m.mov2 = 0;
        m.pc3 = 0;
        m.mov3 = 0;
        return m;
    }
        
    m.pc2 = (int)((code >> pc2_code_shift) & 0xf);
    m.mov2 = (1ULL << ((code >> mov2_1_code_shift) & 0x3f)) | ((1ULL << ((code >> mov2_2_code_shift) & 0x3f)) & ~1ULL);
    
    if (m.type != CAPTURE_PROMOTE){
        m.pc3 = 0;   
        m.mov3 = 0;   
        return m;
    }

    m.pc3 = (int)((code >> pc3_code_shift) & 0xf);
    m.mov3 = (1ULL << ((code >> mov3_code_shift) & 0x3f));

    return m;
}

// add position to transposition table
void add_item(Move* in_m, int type, int depth, uint64_t* board){
    uint64_t hash = get_hash(board);
    
    Node* node = TransTable[hash & 0xffff];
    Node* new_node = malloc(sizeof(Node));

    new_node->move_code = encrypt_move(in_m);
    new_node->next = node;
    new_node->hash = hash;
    new_node->search_info = depth | (type << 6);
    
    TransTable[hash & 0xffff] = new_node;

    if (node == NULL) return;
    
    Node* prev = NULL;
    Node* current = node;
    int count = 0;
    
    // Find the node at position MAX_LINKED_LIST-1
    while (current != NULL && count < MAX_LINKED_LIST) {
        prev = current;
        current = current->next;
        count++;
    }
    
    // Remove Node
    if (current != NULL && prev != NULL) {
        prev->next = NULL; // Cut the link
        free(current);     // Free the excess node
    }
}

// lookup a board positions in the transposition table 
Node* query_table(uint64_t* board){
    uint64_t hash = get_hash(board);
    Node* node = TransTable[hash & 0xffff];
    while(node != NULL){
        if (node->hash == hash) return node;
        node = node->next;
    }
    return NULL;
}

