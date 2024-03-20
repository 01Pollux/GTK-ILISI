#pragma once

#include "ChessTable.h"

/// <summary>
/// Function returns 0 if the king can still escape
/// returns 1 if the king is in stalemate
/// returns 2 if the king was checkmated
/// </summary>
int Chess_KingHasLost(
    ChessTable* Table);

int Chess_X_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_Pawn_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_Knight_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_Bishop_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_Rook_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_Queen_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);

int Chess_King_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y);
