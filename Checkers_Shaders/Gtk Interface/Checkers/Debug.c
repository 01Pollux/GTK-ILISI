#include "Debug.h"

void DCheckerTable_Simulate(CheckerTable* Table, char Mat[8][8])
{
}

void DCheckerTable_Add(CheckerTable* Table, CheckerPieceTeam Team, int X, int Y, gboolean IsQueen)
{
    CheckerPiece* Piece = &Table->Active.Pieces.Mat[X][Y];
    if (Piece->Index == -1)
    {
        int Index = -1;
        CheckerPieceRel* Rel = Team == CPT_White ? Table->Active.WhitePieces : Table->Active.BlackPieces;
        for (int i = 0; i < PIECE_PER_PLAYER; i++, Rel++)
        {
            if (!CheckerPieceRel_Exists(Rel))
            {
                Index = i;
                break;
            }
        }
        if (Index == -1)
        {
            printf("Erreur, Essayer d'ajouter une piece alors que le tableau est plein\n");
            return;
        }

        Rel->Pos.X = X;
        Rel->Pos.Y = Y;

        Piece->Team = Team;
        Piece->Index = Index;
        Piece->IsQueen = IsQueen;

        if (IsQueen)
        {
            Rel->Image = gtk_image_new_from_file(Team == CPT_White ? PIECE_IMAGE_WHITE : PIECE_IMAGE_BLACK);
        }
        else
        {
            Rel->Image = gtk_image_new_from_file(Team == CPT_White ? PIECE_IMAGE_WHITE_Q : PIECE_IMAGE_BLACK_Q);
        }

        gtk_fixed_put(GTK_FIXED(Table->Fixed), Rel->Image, X * 100, Y * 100);
    }
}

void DCheckerTable_Remove(CheckerTable* Table, CheckerPieceTeam Team, int X, int Y)
{
    CheckerPiece* Piece = &Table->Active.Pieces.Mat[X][Y];
    if (Piece->Index != -1)
    {
        CheckerPieceRel* Rel = Piece->Team == CPT_White ? &Table->Active.WhitePieces[Piece->Index] : &Table->Active.BlackPieces[Piece->Index];
        gtk_container_remove(GTK_CONTAINER(Table->Fixed), Rel->Image);
        Rel->Pos.X = Rel->Pos.Y = -1;
        Piece->Index = -1;
    }
}
