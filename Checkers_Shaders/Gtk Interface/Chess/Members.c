
#include "Logic.h"

int Chess_KingHasLost(
    ChessTable* Table)
{
    // Get king piece
    ChessPieceList* KingPiece = Table->PlayerTurn == CTT_White ? Table->WhitePieces : Table->BlackPieces;
    while (KingPiece && (KingPiece->Type != CPT_King))
    {
        KingPiece = KingPiece->Next;
    }

    ChessPieceList* Enemy = Table->PlayerTurn == CTT_White ? Table->BlackPieces : Table->WhitePieces;

    char EscapeDirs[3][3];
    memset(EscapeDirs, 1, sizeof(EscapeDirs));

    char KingX = KingPiece->XPos;
    char KingY = KingPiece->YPos;

    while (Enemy)
    {
        printf("Enemy= %i (%i,%i)\n", Enemy->Type, Enemy->XPos, Enemy->YPos);
        for (char Y = 0; Y < 3; Y++)
        {
            for (char X = 0; X < 3; X++)
            {
                if (!EscapeDirs[Y][X])
                    continue;

                char CurX = KingX + X - 1;
                char CurY = KingY + Y - 1;

                if (!((0 <= CurX) && (CurX < 8)) ||
                    !((0 <= CurY) && (CurY < 8)))
                {
                    printf("Out range: (%i, %i)\n", CurX, CurY);
                    EscapeDirs[Y][X] = 0;
                }
                else
                {
                    if (Chess_X_Attack(Table, Enemy, CurX, CurY))
                    {
                        printf("Check: (%i, %i)\n", CurX, CurY);
                        EscapeDirs[Y][X] = 0;
                    }
                }
            }
        }
        Enemy = Enemy->Next;
    }

    printf("\n\n");

    for (size_t Y = 0; Y < 3; Y++)
    {
        for (size_t X = 0; X < 3; X++)
        {
            if (EscapeDirs[Y][X] == 1)
                return 0;
        }
    }

    return (EscapeDirs[1][1] == 1) ? 1 : 2;
}

// Query to check if the king will be in danger
// if the current piece moved to the wanted position
int Chess_KingIsSafe(
    ChessTable* Table,
    char        X,
    char        Y)
{
    // Get king piece
    ChessPieceList* KingPiece = Table->PlayerTurn == CTT_White ? Table->WhitePieces : Table->BlackPieces;
    while (KingPiece && (KingPiece->Type != CPT_King))
    {
        KingPiece = KingPiece->Next;
    }

    ChessPieceList* Enemy = Table->PlayerTurn == CTT_White ? Table->BlackPieces : Table->WhitePieces;

    char EscapeDirs[3][3];
    memset(EscapeDirs, 1, sizeof(EscapeDirs));

    char KingX = KingPiece->XPos;
    char KingY = KingPiece->YPos;

    while (Enemy)
    {
        for (char Y = 0; Y < 3; Y++)
        {
            for (char X = 0; X < 3; X++)
            {
                if (!EscapeDirs[Y][X])
                    continue;

                char CurX = KingX + X;
                char CurY = KingY + Y;

                if (!((0 <= CurX) && (CurX < 8)) ||
                    !((0 <= CurY) && (CurY < 8)))
                {
                    EscapeDirs[Y][X] = 0;
                }
                else
                {
                    if (Chess_X_Attack(Table, Enemy, CurX, CurY))
                    {
                        EscapeDirs[Y][X] = 0;
                    }
                }
            }
        }
        Enemy = Enemy->Next;
    }

    char RelX = abs(KingX - X);
    char RelY = abs(KingY - Y);

    // Check if we are being checkmated
    return 0;
}

//

int Chess_X_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    switch (Piece->Type)
    {
    case CPT_Pawn:
        return Chess_Pawn_Attack(Table, Piece, X, Y);
    case CPT_Knight:
        return Chess_Knight_Attack(Table, Piece, X, Y);
    case CPT_Bishop:
        return Chess_Bishop_Attack(Table, Piece, X, Y);
    case CPT_Rook:
        return Chess_Rook_Attack(Table, Piece, X, Y);
    case CPT_Queen:
        return Chess_Queen_Attack(Table, Piece, X, Y);
    case CPT_King:
        return Chess_King_Attack(Table, Piece, X, Y);
    }
    return 0;
}

//

int Chess_Pawn_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_Pawn);

    int MoveDir = (int)Piece->Team;
    int Delta   = MoveDir * (Y - Piece->YPos);
    if (Delta != 1)
    {
        return 0;
    }

    Delta       = MoveDir * (X - Piece->XPos);
    int CanMove = (Delta == 1) || (Delta == -1);

    if (!CanMove)
        return 0;

    // Check if the king is surrounded
    return 1;
}

//

int Chess_Knight_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_Knight);

    int YDelta = (Y - Piece->YPos);
    int XDelta = (X - Piece->XPos);

    int CanMove = 0;

    if ((YDelta == -2) || (YDelta == 2))
    {
        CanMove = (XDelta == -1) || (XDelta == 1);
    }
    else if ((XDelta == -2) || (XDelta == 2))
    {
        CanMove = (YDelta == -1) || (YDelta == 1);
    }

    if (!CanMove)
        return 0;

    // Check if the king is in danger

    return 1;
}

//

int Chess_Bishop_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_Bishop);

    int YDelta = (Y - Piece->YPos);
    int XDelta = (X - Piece->XPos);

    if ((YDelta != XDelta) && (YDelta != -XDelta))
        return 0;

    // Check if no piece is in the way
    int StepCount = abs(YDelta);
    int XStep     = XDelta < 0 ? -1 : 1;
    int YStep     = YDelta < 0 ? -1 : 1;

    for (int i = 1; i <= StepCount; i++)
    {
        char CurX = Piece->XPos + i * XStep;
        char CurY = Piece->YPos + i * YStep;

        if (Table->Pieces[CurY][CurX] &&
            Table->Pieces[CurY][CurX]->Team == Piece->Team)
        {
            return 0;
        }
    }

    return 1;
}

//

int Chess_Rook_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_Rook);

    int YDelta = (Y - Piece->YPos);
    int XDelta = (X - Piece->XPos);

    return (YDelta == 0) || (XDelta == 0);
}

//

int Chess_Queen_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_Queen);

    int YDelta = (Y - Piece->YPos);
    int XDelta = (X - Piece->XPos);

    // if not diagonal
    if ((YDelta != XDelta) && (YDelta != -XDelta))
    {
        // if straight line
        if ((XDelta * YDelta))
        {
            return 0;
        }
    }

    int StepCount = abs(YDelta);
    int XStep     = XDelta < 0 ? -1 : 1;
    int YStep     = YDelta < 0 ? -1 : 1;

    if (!XDelta)
        XStep = 0;
    if (!YStep)
        YStep = 0;

    for (int i = 1; i <= StepCount; i++)
    {
        char CurX = Piece->XPos + i * XStep;
        char CurY = Piece->YPos + i * YStep;

        if (Table->Pieces[CurY][CurX] &&
            Table->Pieces[CurY][CurX]->Team == Piece->Team)
        {
            return 0;
        }
    }

    return 1;
}

//

int Chess_King_Attack(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    g_assert(Piece->Type == CPT_King);

    int YDelta = (Y - Piece->YPos);
    int XDelta = (X - Piece->XPos);

    if (YDelta == 0)
    {
        YDelta = 1;
    }
    else if (XDelta == 0)
    {
        XDelta = 1;
    }

    int Delta = abs(YDelta * XDelta);
    return Delta <= 1;
}
