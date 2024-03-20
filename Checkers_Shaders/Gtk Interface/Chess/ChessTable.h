#pragma once

#include "../Creator/Widgets.h"

typedef enum
{
    CTT_SwapTurn = -1,

    CTT_White = -1,
    CTT_Black = 1,

    CTT_Unk
} ChessTurnType;

typedef enum
{
    CPT_Pawn,
    CPT_Knight,
    CPT_Bishop,
    CPT_Rook,
    CPT_Queen,
    CPT_King
} ChessPieceType;

typedef struct ChessPieceList
{
    char           HasMoved;
    char           XPos, YPos;
    ChessTurnType  Team;
    ChessPieceType Type;
    GtkWidget*     Image;

    struct ChessPieceList* Next;
} ChessPieceList;

typedef struct
{
    uint8_t*   ColorTable;
    GtkWidget* ChessImage;
    GtkFixed*  MainBox;

    uint32_t CellWidth;
    uint32_t CellHeight;

    ChessPieceList* WhitePieces;
    ChessPieceList* BlackPieces;

    ChessPieceList* Pieces[8][8];

    ChessPieceList* ActivePiece;
    ChessTurnType   PlayerTurn;

    guint32 ClickTime;
    char    EndGame;
} ChessTable;

typedef enum
{
    // Get piece of same team as current player turn
    CGT_SameTeam,
    // Get piece of opposite team as current player turn
    CGT_OppositeTeam,
    // Get any piece
    CGT_Any
} ChessGetPieceType;

//

void Chess_CreateTable(
    ChessTable*      Table,
    WidgetContainer* MainBox,
    uint8_t          FirstColor[3],
    uint8_t          SecondColor[3],
    uint32_t         TableWidth,
    uint32_t         TableHeight);

void Chess_UpdateTable(
    ChessTable* Table);

ChessPieceList* Chess_GetPiece(
    ChessTable*       Table,
    ChessGetPieceType Type,
    char              X,
    char              Y,
    int*              SameTeam);

//

void Chess_ProcessInput(
    ChessTable* Table,
    char        X,
    char        Y);

//

void Chess_Debug_AddPiece(
    ChessTable*    Table,
    char           IsBlack,
    ChessPieceType Type,
    char           X,
    char           Y);

void Chess_Debug_MoveKing(
    ChessTable* Table,
    char        IsBlack,
    char        X,
    char        Y);
