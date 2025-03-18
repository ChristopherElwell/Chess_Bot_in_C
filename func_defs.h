#ifndef FUNC_DEFS_H
#define FUNC_DEFS_H

#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
     int num_xors;
     unsigned long long int mov1, mov2, mov3;
     int pc1, pc2, pc3;
     unsigned long long int info;
} Move;

#endif // FUNC_DEFS_H
