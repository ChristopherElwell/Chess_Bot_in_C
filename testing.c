#include "constants.h"
#include "get_moves.h"
#include "hash_table.h"

void hash_testing(char* FEN){
    uint64_t* board = from_FEN(FEN);
    Move movs[MOVES_ARRAY_LENGTH];
    get_white_moves(movs,board);
    int pc1match = 0;
    int pc2match = 0;
    int pc3match = 0;
    int mov1match = 0;
    int mov2match = 0;
    int mov3match = 0;
    int infomatch = 0;
    int typematch = 0;
    int total = 0;
    uint64_t code = encrypt_move(movs);
    // print_bit_board(code);
    // print_move(movs);
    // return;
    for (Move* mov = movs; mov->type != BOOK_END; mov++){
        uint64_t code = encrypt_move(mov);
        Move copy = decrypt_move(code);
        total++;
        if(mov->pc1 == copy.pc1){
            pc1match++;
        }
        if(mov->pc2 == copy.pc2){
            pc2match++;
        }
        if(mov->pc3 == copy.pc3){
            pc3match++;
        } else {
            printf("\n============\n");
            print_bit_board(mov->pc3);
            print_bit_board(copy.pc3);
            print_bit_board(code);
        }
        if(mov->mov1 == copy.mov1){
            mov1match++;
        }
        if(mov->mov2 == copy.mov2){
            mov2match++;
        }
        if(mov->mov3 == copy.mov3){
            mov3match++;
        } 
        if(mov->info == copy.info){
            infomatch++;
        } else {
            printf("\n============\n");
            print_bit_board(mov->info);
            print_bit_board(copy.info);
            print_bit_board(code);
        }
        if(mov->type == copy.type){
            typematch++;
        }
    } 

    // Only print matches that are not equal to total
    if (pc1match != total) printf("\npc1 %d != %d | %s", total, pc1match, FEN);
    if (pc2match != total) printf("\npc2 %d != %d | %s", total, pc2match, FEN);
    if (pc3match != total) printf("\npc3 %d != %d | %s", total, pc3match, FEN);
    if (mov1match != total) printf("\nmov1 %d != %d | %s", total, mov1match, FEN);
    if (mov2match != total) printf("\nmov2 %d != %d | %s", total, mov2match, FEN);
    if (mov3match != total) printf("\nmov3 %d != %d | %s", total, mov3match, FEN);
    if (typematch != total) printf("\ntype %d != %d | %s", total, typematch, FEN);
    if (infomatch != total) printf("\ninfo %d != %d | %s", total, infomatch, FEN);}