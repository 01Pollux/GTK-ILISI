#pragma once

#include "Checker.h"

// 0 == none
// 1 == white
// 2 == black
// 3 == white queen
// 4 == black queen
void DCheckerTable_Simulate(
    CheckerTable* Table,
    char Mat[8][8]);

void DCheckerTable_Add(
    CheckerTable* Table,
    CheckerPieceTeam Team,
    int X,
    int Y,
    gboolean IsQueen);

void DCheckerTable_Remove(
    CheckerTable* Table,
    CheckerPieceTeam Team,
    int X,
    int Y);