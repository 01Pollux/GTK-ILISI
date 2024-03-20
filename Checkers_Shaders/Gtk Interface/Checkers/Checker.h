#pragma once

#include "../Creator/Widgets.h"
#include "Net/Networking.h"

#define CHECKER_TABLE_X 8
#define CHECKER_TABLE_Y 8

#define CHECKER_TABLE_COLOR_WHITE 255, 255, 255
#define CHECKER_TABLE_COLOR_BLACK 0, 0, 0

#define CHECKER_PIECE_COUNT 12
#define CHECKER_PIECE_SIZE  80

#define CHECKER_PIECE_IMG_WHITE       "Checkers/pred.png"
#define CHECKER_PIECE_IMG_BLACK       "Checkers/pblack.png"
#define CHECKER_PIECE_IMG_WHITE_QUEEN "Checkers/predq.png"
#define CHECKER_PIECE_IMG_BLACK_QUEEN "Checkers/pblackq.png"
#define CHECKER_PIECE_IMG_FUTURE      "Checkers/predf.png"

//

typedef struct
{
    char X, Y;
} CPosition;

typedef enum
{
    CPT_Empty,
    CPT_White,
    CPT_Black,
} CheckerPieceTeam;

typedef enum
{
    CPM_PlayerVsPlayer,
    CPM_PlayerVsBot,
    CPM_BotVsBot,
    CPM_PlayerVsRPC
} CheckerPieceMode;

typedef enum
{
    CPD_Easy = 2,
    CPD_Medium = 4,
    CPD_Hard = 6,
    CPD_Expert = 8,
} CheckerPieceDifficulty;

typedef struct
{
    CPosition Pos;
    GtkWidget* Image;
} CheckerPieceRel;

typedef struct
{
    char Index;   // Index in the WhitePieces or BlackPieces array
    char IsQueen; // Is the piece a queen
    char WasQueen;
    CheckerPieceTeam Team; // Team of the piece
} CheckerPiece;

// Reserve au future
// On est besoin d'avoir une structure contennant matrice des pieces pour la modifier
// recursivement pour traiter les conditions de future
// Sinon, on va avoir une passage addresse qui va changer la matrice final
typedef struct
{
    CheckerPiece Mat[CHECKER_TABLE_Y][CHECKER_TABLE_X];
} CheckerPieceMatrix;

typedef struct
{
    CheckerPieceRel WhitePieces[CHECKER_PIECE_COUNT];
    CheckerPieceRel BlackPieces[CHECKER_PIECE_COUNT];
    CheckerPieceMatrix Pieces;
} CheckerPieceInfo;

typedef struct CheckerPositionTree
{
    CPosition* TopLeft;
    CPosition* TopRight;
    CPosition* BottomLeft;
    CPosition* BottomRight;

    int TopLeftCount;
    int TopRightCount;
    int BottomLeftCount;
    int BottomRightCount;
} CheckerPositionTree;

typedef struct CheckerPieceHistory
{
    CheckerPieceInfo Table;
    struct CheckerPieceHistory* Next;
} CheckerPieceHistory;

typedef struct CheckerTable
{
    GtkWindow* Window;
    GtkFixed* Fixed;
    GtkWidget* ScoreLabel;
    GtkWidget* TimerLabel;

    CheckerPieceHistory* Snapshot;
    CheckerPieceHistory* Old;
    gboolean FirstRewind;

    gboolean Paused;
    CheckerPieceInfo Active;
    CheckerPieceInfo Final;

    CheckerPieceRel* ActivePiece;

    GtkWidget* FuturePiece;
    CheckerPositionTree ActivePieceFutures;

    guint32 NumberOfTurns;
    guint32 NumberOfSeconds[2];

    CheckerPieceTeam Turn;
    CheckerPieceTeam Team;

    CheckerPieceMode Mode;
    CheckerPieceDifficulty Difficulty;

    gboolean NeedNetUpdate;
} CheckerTable;

//

typedef enum
{
    CPT_TOP_LEFT,
    CPT_TOP_RIGHT,
    CPT_BOTTOM_LEFT,
    CPT_BOTTOM_RIGHT,
} CPTPosition;

typedef struct
{
    CheckerPositionTree* BestPosition;
    int BestScore;
} CheckerPositionScore;

void CheckerTable_GetFutures(CheckerPieceInfo* Table, CPosition Pos, CheckerPositionTree* Tree, gboolean OnlyEat);

void CheckerPositionTree_Free(CheckerPositionTree* Tree);

//

void CheckerTable_PlaySound(gboolean HardSound);

gboolean CheckerPieceRel_Exists(CheckerPieceRel* Rel);

void CheckerTable_EndGame(CheckerTable* Table, CheckerPieceTeam Team);

void CheckerTable_QueenUpdate(CheckerTable* Table, CheckerPieceRel* Rel);

//

void CheckerTable_Init(CheckerTable* Table, CheckerPieceMode Mode, CheckerPieceDifficulty Difficulty);

void CheckerTable_DestroyFuture(CheckerTable* Table);

int CheckerTable_MovePiece(CheckerPieceInfo* Info, CheckerPieceTeam Team, CPosition From, CPosition To);

void CheckerTable_AbsolutePosition(CPosition Rel, int* PosX, int* PosY);
void CheckerTable_RelativePosition(int PosX, int PosY, CPosition* Rel);

//

int CheckerTable_TrySelect(CheckerTable* Table, CPosition Pos);

//

void CheckerTable_Refresh(CheckerTable* Table);

void CheckerTable_SetTurn(CheckerTable* Table, CheckerPieceTeam Team);
void CheckerTable_SetNextTurn(CheckerTable* Table);
void CheckerTable_NextTurn(CheckerTable* Table);

void CheckerTable_Print(CheckerPieceMatrix* Table);

void CheckerTable_Snapshot(CheckerTable* Table);
void CheckerTable_Undo(CheckerTable* Table);
void CheckerTable_Redo(CheckerTable* Table);

gboolean CheckerTable_TryEndGame(CheckerTable* Table);
