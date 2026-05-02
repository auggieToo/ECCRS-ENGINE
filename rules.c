//---------------------------------------------
//Warning: This file was auto generated
//Changing this file may lead to unexpected behavior
//---------------------------------------------


#include "rules.h"


Rule ruleset[] = {
    {0, {{1,1}}, 1, 1},
    {1, {{1,1},{4,1}}, 2, 0},
    {2, {{1,1},{4,1},{5,1}}, 3, 1},
    {3, {{2,1},{3,1}}, 2, 1},
    {4, {{2,1},{3,1},{4,1}}, 3, 0},
};

Instance F = { {{1,0},{2,1},{3,1},{4,0},{5,0}}, 5};
 const unsigned int SIZE_OF_RULESET = 5;
