#include "Logic.h"

//

void Chess_EndGame(
    ChessTable*   Table,
    ChessTurnType Winner)
{
    switch (Winner)
    {
    case CTT_Black:
        printf("Checkmate! Black is the winner.\n");
        break;
    case CTT_White:
        printf("Checkmate! White is the winner.\n");
        break;
    case CTT_Unk:
        printf("Stalemate.\n");
        break;
    }

    Table->EndGame = 1;

    const size_t TextureSize = (Table->CellWidth * 8) * (Table->CellHeight * 8) * 4;
    for (int Pixel = 0; Pixel < TextureSize; Pixel += 4)
    {
        Table->ColorTable[Pixel]     = (uint8_t)(Table->ColorTable[Pixel] * 1.75f);
        Table->ColorTable[Pixel + 1] = Table->ColorTable[Pixel + 2] = 0x00;
    }

    gtk_widget_activate(Table->ChessImage);
}

//

void Chess_SetPiecePosition(
    ChessTable* Table,
    char        FromX,
    char        FromY,
    char        ToX,
    char        ToY)
{
    Table->Pieces[ToY][ToX]     = Table->Pieces[FromY][FromX];
    Table->Pieces[FromY][FromX] = NULL;

    ChessPieceList* Piece = Table->Pieces[ToY][ToX];

    Piece->XPos = ToX;
    Piece->YPos = ToY;

    int AbsX = Piece->XPos * Table->CellWidth;
    int AbsY = Piece->YPos * Table->CellHeight;

    gtk_fixed_move(Table->MainBox, Piece->Image, AbsX, AbsY);
}

// TODO
int Chess_InCheckmate(ChessTable* Table)
{
    return 0;
}

//

static void Chess_NextTurn(
    ChessTable* Table)
{
    Table->PlayerTurn *= CTT_SwapTurn;
    if (Chess_InCheckmate(Table))
    {
        Chess_EndGame(Table, Table->PlayerTurn == CTT_White);
    }
}

//

static void Chess_AdvancePiece(
    ChessTable* Table,
    char        X,
    char        Y)
{
    ChessPieceList* ActivePiece = Table->ActivePiece;

    ActivePiece->HasMoved = 1;

    Chess_SetPiecePosition(
        Table,
        ActivePiece->XPos,
        ActivePiece->YPos,
        X,
        Y);

    Chess_NextTurn(Table);
}

//

static void Chess_TryMoveActivePiece(
    ChessTable* Table,
    char        X,
    char        Y)
{
    ChessPieceList* ActivePiece = Table->ActivePiece;

    switch (ActivePiece->Type)
    {
    case CPT_Pawn:
    {
        if (ActivePiece->XPos != X)
        {
            return;
        }

        int MoveDir = (int)Table->PlayerTurn;
        int Delta   = MoveDir * (Y - ActivePiece->YPos);

        if (ActivePiece->HasMoved)
        {
            if (Delta > 1)
            {
                return;
            }
        }
        else if (Delta > 2)
        {
            return;
        }
        break;
    }
    default:
    {
        if (!Chess_X_Attack(Table, Table->ActivePiece, X, Y))
        {
            return;
        }
        break;
    }
    }

    Chess_AdvancePiece(Table, X, Y);
}

//

ChessPieceList* Chess_RemovePieceFromList(
    ChessPieceList* List,
    ChessPieceList* Element)
{
    if (List == Element)
    {
        List = List->Next;
    }
    else
    {
        ChessPieceList* Tmp = List;
        while (Tmp->Next && (Tmp->Next != Element))
        {
            Tmp = Tmp->Next;
        }
        Tmp->Next = Element->Next;
    }

    free(Element);
    return List;
}

//

void Chess_DigestPiece(
    ChessTable* Table,
    char        X,
    char        Y)
{
    ChessPieceList* Piece = Table->Pieces[Y][X];
    Table->Pieces[Y][X]   = NULL;

    gtk_container_remove(GTK_CONTAINER(Table->MainBox), Piece->Image);
    switch (Table->PlayerTurn)
    {
    case CTT_Black:
    {
        Table->WhitePieces = Chess_RemovePieceFromList(Table->WhitePieces, Piece);
        break;
    }
    case CTT_White:
    {
        Table->BlackPieces = Chess_RemovePieceFromList(Table->BlackPieces, Piece);
        break;
    }
    }
}

//

//

void Chess_TryAttackPiece(
    ChessTable*     Table,
    ChessPieceList* Piece,
    char            X,
    char            Y)
{
    if (Chess_X_Attack(Table, Table->ActivePiece, X, Y))
    {
        if (Table->Pieces[Y][X]->Type != CPT_King)
        {
            Chess_DigestPiece(Table, X, Y);
            Chess_AdvancePiece(Table, X, Y);
        }
    }
}

//

void Chess_ProcessInput(
    ChessTable* Table,
    char        X,
    char        Y)
{

    int SameTeam;

    ChessPieceList* Piece = Chess_GetPiece(Table, CGT_Any, X, Y, &SameTeam);
    if (!Piece)
    {
        Chess_TryMoveActivePiece(Table, X, Y);
    }
    else if (SameTeam)
    {
        Table->ActivePiece = Piece;
        return;
    }
    else
    {
        Chess_TryAttackPiece(Table, Piece, X, Y);
    }

    /*  int Stat = Chess_KingHasLost(Table);
      switch (Stat)
      {
      case 1:
          Chess_EndGame(Table, CTT_Unk);
          break;
      case 2:
          Chess_EndGame(Table, -Table->PlayerTurn);
          break;
      default:
          break;
      }*/
    Table->ActivePiece = NULL;
}
