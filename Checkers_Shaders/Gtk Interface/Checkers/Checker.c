#include <gdk/gdkkeysyms.h>
#include "Checker.h"
#include "Bot.h"
#include <Windows.h>
#include <mmsystem.h>

/*
* Entr�e: La table de jeu et la nouvelle position
* Sortie: Un boolean
* Fonction: Cette fonction v�rifie si une position donn�e
peut �tre travers�e dans le future.
Elle parcourt les diff�rentes positions possibles
dans les diff�rentes directions
(en haut � gauche, en haut � droite,
en bas � gauche et en bas � droite)
et v�rifie si la position donn�e correspond � l'une de ces positions.
*/
static gboolean CheckerTable_CanTraverseCurrentFuture(CheckerTable* Table, CPosition NewPos)
{
    // Pour tout les pieces actives haut gauche
    for (int i = 0; i < Table->ActivePieceFutures.TopLeftCount; i++)
    {
        // On verifie si les deux cordonn�es nouvelles des pieces est
        // Egale a la nouvelle position
        if (Table->ActivePieceFutures.TopLeft[i].X == NewPos.X && Table->ActivePieceFutures.TopLeft[i].Y == NewPos.Y)
        {
            return TRUE;
        }
    }
    // Pour tout les pieces actives haut droit
    for (int i = 0; i < Table->ActivePieceFutures.TopRightCount; i++)
    {
        if (Table->ActivePieceFutures.TopRight[i].X == NewPos.X && Table->ActivePieceFutures.TopRight[i].Y == NewPos.Y)
        {
            return TRUE;
        }
    }
    // Pour tout les pieces actives bas gauche
    for (int i = 0; i < Table->ActivePieceFutures.BottomLeftCount; i++)
    {
        if (Table->ActivePieceFutures.BottomLeft[i].X == NewPos.X && Table->ActivePieceFutures.BottomLeft[i].Y == NewPos.Y)
        {
            return TRUE;
        }
    }
    // Pour tout les pieces actives bas droit
    for (int i = 0; i < Table->ActivePieceFutures.BottomRightCount; i++)
    {
        if (Table->ActivePieceFutures.BottomRight[i].X == NewPos.X && Table->ActivePieceFutures.BottomRight[i].Y == NewPos.Y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
* Entree:
* Sortie:
* Fonction: Cette fonction d�place une image future
(repr�sentant un mouvement potentiel) sur une table de jeu.
Elle utilise les coordonn�es absolues
fournies pour d�placer l'image future sur le tableau.
*/
static void CheckerTable_MoveFutureImage(CheckerTable* Table,
                                         CPosition Pos)
{
    int AbsX, AbsY;
    // Recuperation de la position absolue
    CheckerTable_AbsolutePosition(Pos, &AbsX, &AbsY);
    // Changement de l'image � l'image future
    gtk_fixed_move(Table->Fixed, Table->FuturePiece, AbsX, AbsY);
}

/*
* Entr�e:
* Sortie:
* Fonction: Cette fonction d�truit
une image future et cache le widget associ�.
*/
void CheckerTable_DestroyFuture(CheckerTable* Table)
{
    CheckerPositionTree_Free(&Table->ActivePieceFutures);
    gtk_widget_hide(Table->FuturePiece);
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction v�rifie si une
 pi�ce peut manger une autre pi�ce
 dans le chemin entre deux positions donn�es.
 Elle utilise les coordonn�es de d�part et d'arriv�e
 pour d�terminer la direction du mouvement.
 Elle parcourt ensuite les positions interm�diaires
 et v�rifie si une pi�ce peut �tre mang�e.
 Elle renvoie un score bas� sur
 le nombre de pi�ces mang�es et le type de pi�ces.
 */
int CheckerTable_EatInPath(CheckerPieceInfo* Info, CheckerPieceTeam Team, CPosition From, CPosition To)
{
    // Difference de position
    CPosition Delta = {
        To.X - From.X,
        To.Y - From.Y
    };

    // Calcule de position absolue
    Delta.X /= abs(Delta.X);
    Delta.Y /= abs(Delta.Y);

    int Score = 0;
    gboolean BeQueen = FALSE;

    // Verification de l'equipe
    CheckerPieceRel* Rel = Team == CPT_White ? &Info->BlackPieces[0] : &Info->WhitePieces[0];
    char X = From.X + Delta.X, Y = From.Y + Delta.Y;

    // Tant que la position n'est pas la position d'arriv�e
    while (X != To.X && Y != To.Y)
    {
        // On recupere l'info de la case de position X,Y
        CheckerPiece* Piece = &Info->Pieces.Mat[Y][X];

        // Si c'est une case vide
        if (Piece->Team == CPT_Empty)
        {
            // On peut continuer
            Score++;

            X += Delta.X;
            Y += Delta.Y;
            continue;
        }
        else if (Piece->IsQueen)
        {
            // Si la piece est une dame le score est mieux
            Score += 3;
        }
        else
        {
            // Si c'est un pion le score est normal
            Score += 2;
        }

        // Retrait de la piece de la table
        Rel[Piece->Index].Pos.X = -1;
        Rel[Piece->Index].Pos.Y = -1;

        // La case deviendera libre
        Info->Pieces.Mat[Y][X].Team = CPT_Empty;

        // Changement de la case de pion en jeu
        X += Delta.X;
        Y += Delta.Y;
    }

    // Retour de score
    return Score;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction effectue un d�placement
 d'une pi�ce d'une position donn�e � une position donn�e.
 Elle utilise CheckerTable_EatInPath pour v�rifier
 si des pi�ces doivent �tre mang�es dans le chemin du d�placement.
 Elle met � jour les informations de la pi�ce et retourne un score
 bas� sur les pi�ces mang�es et les changements de statut
 (par exemple, une pi�ce devient une reine).
 */
int CheckerTable_MovePiece(CheckerPieceInfo* Info,
                           CheckerPieceTeam Team,
                           CPosition From, CPosition To)
{
    // On r�cup�re le score
    int Score = CheckerTable_EatInPath(Info, Team, From, To);

    // D�place la pi�ce de la position From � la position To dans la matrice des pi�ces
    Info->Pieces.Mat[To.Y][To.X] = Info->Pieces.Mat[From.Y][From.X];
    Info->Pieces.Mat[From.Y][From.X].Team = CPT_Empty;

    // R�cup�re un pointeur vers la pi�ce d�plac�e
    CheckerPiece* Piece = &Info->Pieces.Mat[To.Y][To.X];
    CheckerPieceRel* Rel;

    // V�rifie si la pi�ce �tait une reine avant le d�placement
    gboolean WasQueen = Piece->IsQueen;

    // Met � jour la position de la pi�ce relative � son �quipe (blanche ou noire)
    if (Piece->Team == CPT_White)
    {
        Rel = &Info->WhitePieces[Piece->Index];
        // Si la pi�ce atteint la rang�e 0, elle devient une reine
        if (To.Y == 0)
        {
            Piece->IsQueen = TRUE;
        }
    }
    else
    {
        Rel = &Info->BlackPieces[Piece->Index];
        // Si la pi�ce atteint la rang�e 7, elle devient une reine
        if (To.Y == 7)
        {
            Piece->IsQueen = TRUE;
        }
    }

    // Si le statut de reine de la pi�ce a chang�, ajoute 4 au score
    if (Piece->IsQueen != WasQueen)
    {
        Score += 4;
    }

    // Met � jour la position relative de la pi�ce
    Rel->Pos.X = To.X;
    Rel->Pos.Y = To.Y;

    return Score;
}

void CheckerTable_PlaySound(gboolean HardSound)
{
    if (HardSound)
        PlaySound(TEXT("Checkers\\capture.wav"), NULL, SND_FILENAME | SND_ASYNC);
    else
        PlaySound(TEXT("Checkers\\move-self.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

// MAKING THE MOVE HERE:
/*
 * Entr�e: Fenetre GTK, Evenement, la table de jeu
 * Sortie: boolean
 * Fonction: Cette fonction est appel�e
 lorsqu'un bouton de la table est press�.
 Elle g�re les interactions du joueur avec la table.
 Selon le mode de jeu et l'�tat actuel de la table,
 elle effectue diff�rentes actions, comme la s�lection d'une pi�ce,
 le d�placement d'une pi�ce ou le changement de tour.
 */
gboolean CheckerTable_OnTablePress(GtkWidget* Window,
                                   GdkEventButton Event,
                                   CheckerTable* Table)
{
    if (Table->Mode == CPM_BotVsBot)
        return FALSE;

    if (Table->Mode == CPM_PlayerVsRPC && Table->Team != Table->Turn)
        return FALSE;

    if (Event.type != GDK_BUTTON_RELEASE || Table->Paused)
        return FALSE;

    CPosition NewPos;
    CheckerTable_RelativePosition((int)Event.x - 50,
                                  (int)Event.y - 100,
                                  &NewPos);

    if (NewPos.X < 0 || NewPos.Y < 0)
        return FALSE;

    if (Table->ActivePiece)
    {
        // Si une pi�ce est d�j� active, on traite le d�placement
        CheckerPieceTeam Team = Table->Active.Pieces.Mat[NewPos.Y][NewPos.X].Team;
        if (Team == CPT_Empty)
        {
            gboolean CanTraverse = CheckerTable_CanTraverseCurrentFuture(Table, NewPos);
            if (CanTraverse)
            {
                Table->NumberOfTurns++;
                int Score = CheckerTable_MovePiece(&Table->Active, Table->Turn, Table->ActivePiece->Pos, NewPos);
                CheckerTable_PlaySound(Score > 0);
                CheckerTable_DestroyFuture(Table);
                if (Score > 0)
                {
                    CheckerTable_GetFutures(&Table->Active, Table->ActivePiece->Pos, &Table->ActivePieceFutures, TRUE);
                    gtk_widget_show(Table->FuturePiece);
                }
                CheckerTable_Refresh(Table);
            }
            return TRUE;
        }
        else if (!(Table->Turn == Team && !Table->NumberOfTurns))
        {
            // Si la pi�ce appartient � l'adversaire ou si ce n'est pas le premier mouvement du joueur actif, on ignore l'action
            return TRUE;
        }
        // Traitement du changement de pi�ce active
    }

    if (CheckerTable_TrySelect(Table, NewPos))
    {
        // Si la s�lection d'une nouvelle pi�ce est r�ussie, on d�truit les mouvements futurs et on r�cup�re les futures positions possibles pour la nouvelle pi�ce active
        CheckerTable_DestroyFuture(Table);
        CheckerTable_GetFutures(&Table->Active, Table->ActivePiece->Pos, &Table->ActivePieceFutures, FALSE);

        CheckerTable_MoveFutureImage(Table, NewPos);
        gtk_widget_show(Table->FuturePiece);
    }

    return TRUE;
}

// CHANGING TURNS OR RESET/REWIND
/*
 * Entr�e:Evenement et table
 * Sortie: boolean
 * Fonction:Cette fonction est appel�e lorsqu'une touche est
 press�e sur la table.
 Elle g�re les actions sp�cifiques li�es
 � la touche press�e,
 comme le passage au tour suivant ou
 la r�initialisation/retour en arri�re.
 */
gboolean CheckerTable_OnTableCommit(
    GtkWidget* Window,
    GdkEventKey Event,
    CheckerTable* Table)
{
    if (Event.type != GDK_KEY_PRESS || Table->Turn == CPT_Empty || Table->Paused)
        return FALSE;

    switch (Table->Mode)
    {
    case CPM_PlayerVsPlayer:
    case CPM_PlayerVsBot:
    case CPM_PlayerVsRPC:
    {
        gboolean NextTurn = FALSE;

        switch (Event.keyval)
        {
        case GDK_KEY_e:
        {
            NextTurn = TRUE;
            Table->Final = Table->Active;
            CheckerTable_Snapshot(Table);
            CheckerTable_NextTurn(Table);

            if (CheckerTable_TryEndGame(Table))
            {
                return FALSE;
            }
            break;
        }
        case GDK_KEY_r:
        {
            Table->Active = Table->Final;
            CheckerTable_Refresh(Table);
            break;
        }
        }
    }
    break;
    }

    return FALSE;
}

// DISPLAY THE ACTIVE PIECE:
/*
 * Entr�e:
 * Sortie:
 * Fonction:Cette fonction est appel�e lorsqu'un mouvement de souris
 est d�tect� sur la table.
 Elle g�re le d�placement d'une pi�ce future en fonction de la
 position de la souris.
 Elle v�rifie �galement si une pi�ce peut �tre travers�e
 dans le futur � la position actuelle de la souris.
 */
gboolean CheckerTable_OnMouseMove(GtkWidget* Widget, GdkEventMotion* Event, CheckerTable* Table)
{
    // V�rifier si aucune pi�ce active n'est s�lectionn�e
    if (!Table->ActivePiece)
        return FALSE;

    CPosition Pos;
    // Calculer la position relative de la souris par rapport � la table de jeu
    CheckerTable_RelativePosition((int)Event->x - 50, (int)Event->y - 100, &Pos);

    // V�rifier si la pi�ce active peut traverser vers la position actuelle de la souris
    gboolean ShouldDisplay = CheckerTable_CanTraverseCurrentFuture(Table, Pos);

    // Si la pi�ce peut traverser vers cette position, mettre � jour l'affichage de la future position
    if (ShouldDisplay)
    {
        CheckerTable_MoveFutureImage(Table, Pos);
    }

    return FALSE;
}

//
/*
 * Entr�e:Positions
 * Sortie:Aucune
 * Fonction:  Ces fonctions convertissent les positions absolues (pixels) en positions relatives (cases de la table)
 et vice versa.
 */
void CheckerTable_AbsolutePosition(CPosition Rel, int* PosX, int* PosY)
{
    *PosX = (Rel.X * CHECKER_PIECE_SIZE);
    *PosY = (Rel.Y * CHECKER_PIECE_SIZE);
}
void CheckerTable_RelativePosition(int PosX, int PosY, CPosition* Rel)
{
    Rel->X = (char)(PosX / CHECKER_PIECE_SIZE);
    Rel->Y = (char)(PosY / CHECKER_PIECE_SIZE);
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction tente de s�lectionner
 une pi�ce � partir de la position donn�e.
 Si une pi�ce du joueur correspondant est pr�sente � cette position,
 elle est s�lectionn�e et une valeur bool�enne
 est renvoy�e pour indiquer le succ�s de la s�lection.
 */
gboolean CheckerTable_TrySelect(CheckerTable* Table, CPosition Pos)
{
    // R�cup�rer la pi�ce situ�e � la position donn�e
    CheckerPiece* Piece = &Table->Active.Pieces.Mat[Pos.Y][Pos.X];

    // V�rifier si la pi�ce appartient � l'�quipe en cours de jeu
    if (Piece->Team == Table->Turn)
    {
        // S�lectionner la pi�ce active en fonction de l'�quipe en cours
        if (Table->Turn == CPT_White)
        {
            Table->ActivePiece = &Table->Active.WhitePieces[Piece->Index];
        }
        else
        {
            Table->ActivePiece = &Table->Active.BlackPieces[Piece->Index];
        }

        // V�rifier si la pi�ce active existe dans la structure CheckerPieceRel
        if (CheckerPieceRel_Exists(Table->ActivePiece))
        {
            return TRUE; // La s�lection de la pi�ce est r�ussie
        }
    }

    // R�initialiser la pi�ce active et retourner FALSE pour indiquer l'�chec de la s�lection
    Table->ActivePiece = NULL;
    return FALSE;
}

//

/*
 * Entr�e:
 * Sortie:
 * Fonction:  renvoie un pointeur vers la pi�ce d'origine � partir de
 la matrice des pi�ces, en fonction des coordonn�es fournies.
 */
static CheckerPiece* CheckerTable_GetOrigin(CheckerPieceMatrix* Table, char X, char Y)
{
    return (X < 0 || X >= CHECKER_TABLE_X || Y < 0 || Y >= CHECKER_TABLE_Y) ? NULL : &Table->Mat[Y][X];
}

typedef enum
{
    CTV_OUT_OF_BOUNDS,
    CTV_EMPTY_PLACE,
    CTV_SAME_TEAM,
    CTV_DIFFERENT_TEAM
} CTValidation;

/*
 * Entr�e:
 * Sortie:
 * Fonction: v�rifie si le mouvement d'une pi�ce est valide en v�rifiant
 les cases d'origine et de destination.
 Elle renvoie diff�rentes valeurs
 (CTV_OUT_OF_BOUNDS, CTV_EMPTY_PLACE, CTV_DIFFERENT_TEAM, CTV_SAME_TEAM)
 pour indiquer le r�sultat de la validation.
 */
static CTValidation CheckerTable_Validate(CheckerPieceMatrix* Table, CheckerPiece* Origin, char DestX, char DestY)
{
    // R�cup�rer la pi�ce de destination en fonction des coordonn�es fournies
    CheckerPiece* Dest = CheckerTable_GetOrigin(Table, DestX, DestY);

    // V�rifier si la pi�ce de destination existe
    if (!Dest)
    {
        return CTV_OUT_OF_BOUNDS; // La pi�ce est en dehors des limites de la table
    }

    // V�rifier si la case de destination est vide
    if (Dest->Team == CPT_Empty)
    {
        return CTV_EMPTY_PLACE; // La case de destination est vide
    }

    // V�rifier si la pi�ce de destination est de l'�quipe diff�rente de la pi�ce d'origine
    // Si c'est le cas, cela indique que la pi�ce d'origine peut potentiellement effectuer une capture (manger)
    return (Dest->Team != Origin->Team) ? CTV_DIFFERENT_TEAM : CTV_SAME_TEAM;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: ins�re une position dans un arbre de positions.
 */
void CheckerPositionTree_Insert(CheckerPositionTree* Tree, CPTPosition Position, char X, char Y);

/*
 * Entr�e:
 * Sortie:
 * Fonction: g�re le mouvement basique d'une pi�ce. Selon l'�quipe
 de la pi�ce d'origine,
 elle v�rifie les mouvements possibles vers le haut gauche, le haut droit,
 le bas gauche et le bas droit.
 Si une case est vide, elle l'ajoute � l'arbre de positions.
 */
static void CheckerTable_BasicMovement(CheckerPieceMatrix* Table,
                                       CheckerPiece* Origin,
                                       CheckerPositionTree* Tree,
                                       CPosition Pos)
{
    if (Origin->Team == CPT_White)
    {
        // Top left
        if (CheckerTable_Validate(Table, Origin, Pos.X - 1, Pos.Y - 1) == CTV_EMPTY_PLACE)
        {
            CheckerPositionTree_Insert(Tree, CPT_TOP_LEFT, Pos.X - 1, Pos.Y - 1);
        }
        // Top right
        if (CheckerTable_Validate(Table, Origin, Pos.X + 1, Pos.Y - 1) == CTV_EMPTY_PLACE)
        {
            CheckerPositionTree_Insert(Tree, CPT_TOP_RIGHT, Pos.X + 1, Pos.Y - 1);
        }
    }
    else
    {
        // Bottom left
        if (CheckerTable_Validate(Table, Origin, Pos.X - 1,
                                  Pos.Y + 1) == CTV_EMPTY_PLACE)
        {
            CheckerPositionTree_Insert(Tree, CPT_BOTTOM_LEFT, Pos.X - 1,
                                       Pos.Y + 1);
        }
        // Bottom right
        if (CheckerTable_Validate(Table, Origin, Pos.X + 1, Pos.Y + 1) == CTV_EMPTY_PLACE)
        {
            CheckerPositionTree_Insert(Tree, CPT_BOTTOM_RIGHT, Pos.X + 1,
                                       Pos.Y + 1);
        }
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: g�re la possibilit� de manger une pi�ce.
 Elle v�rifie si une pi�ce peut manger une autre en regardant les cases environnantes.
 Si une pi�ce adverse est pr�sente,
 elle v�rifie si la case apr�s celle-ci est vide et l'ajoute � l'arbre de positions.
 */
static gboolean CheckerTable_BasicEat(CheckerPieceMatrix* Table,
                                      CheckerPiece* Origin,
                                      CheckerPositionTree* Tree,
                                      CPosition Pos)
{
    gboolean HasEaten = FALSE;
    gboolean Break;
    CPosition TmpPos;

    if (Origin->Team == CPT_White)
    {
        Break = FALSE;
        TmpPos = Pos;
        while (!Break)
        {
            TmpPos.X -= 1;
            TmpPos.Y -= 1;
            // Top left
            CTValidation Res = CheckerTable_Validate(Table, Origin,
                                                     TmpPos.X, TmpPos.Y);
            switch (Res)
            {
            case CTV_DIFFERENT_TEAM:
                Res = CheckerTable_Validate(Table, Origin,
                                            TmpPos.X - 1, TmpPos.Y - 1);
                if (Res == CTV_EMPTY_PLACE)
                {
                    TmpPos.X -= 1;
                    TmpPos.Y -= 1;
                    if (Tree)
                    {
                        HasEaten = TRUE;
                        CheckerPositionTree_Insert(Tree, CPT_TOP_LEFT,
                                                   TmpPos.X, TmpPos.Y);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    Break = TRUE;
                }
                break;
            default:
                Break = TRUE;
                break;
            }
        }

        Break = FALSE;
        TmpPos = Pos;
        while (!Break)
        {
            TmpPos.X += 1;
            TmpPos.Y -= 1;
            // Top left
            CTValidation Res = CheckerTable_Validate(Table, Origin,
                                                     TmpPos.X, TmpPos.Y);
            switch (Res)
            {
            case CTV_DIFFERENT_TEAM:
                Res = CheckerTable_Validate(Table, Origin,
                                            TmpPos.X + 1, TmpPos.Y - 1);
                if (Res == CTV_EMPTY_PLACE)
                {
                    TmpPos.X += 1;
                    TmpPos.Y -= 1;
                    if (Tree)
                    {
                        HasEaten = TRUE;
                        CheckerPositionTree_Insert(Tree, CPT_TOP_RIGHT,
                                                   TmpPos.X, TmpPos.Y);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    Break = TRUE;
                }
                break;
            default:
                Break = TRUE;
                break;
            }
        }
    }
    else
    {
        Break = FALSE;
        TmpPos = Pos;
        while (!Break)
        {
            TmpPos.X -= 1;
            TmpPos.Y += 1;
            // Top left
            CTValidation Res = CheckerTable_Validate(Table, Origin,
                                                     TmpPos.X, TmpPos.Y);
            switch (Res)
            {
            case CTV_DIFFERENT_TEAM:
                Res = CheckerTable_Validate(Table, Origin,
                                            TmpPos.X - 1, TmpPos.Y + 1);
                if (Res == CTV_EMPTY_PLACE)
                {
                    TmpPos.X -= 1;
                    TmpPos.Y += 1;
                    if (Tree)
                    {
                        HasEaten = TRUE;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_BOTTOM_LEFT, TmpPos.X, TmpPos.Y);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    Break = TRUE;
                }
                break;
            default:
                Break = TRUE;
                break;
            }
        }

        Break = FALSE;
        TmpPos = Pos;
        while (!Break)
        {
            TmpPos.X += 1;
            TmpPos.Y += 1;
            // Top left
            CTValidation Res = CheckerTable_Validate(Table,
                                                     Origin, TmpPos.X, TmpPos.Y);
            switch (Res)
            {
            case CTV_DIFFERENT_TEAM:
                Res = CheckerTable_Validate(Table, Origin,
                                            TmpPos.X + 1, TmpPos.Y + 1);
                if (Res == CTV_EMPTY_PLACE)
                {
                    TmpPos.X += 1;
                    TmpPos.Y += 1;
                    if (Tree)
                    {
                        HasEaten = TRUE;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_BOTTOM_RIGHT, TmpPos.X, TmpPos.Y);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    Break = TRUE;
                    break;
                }
                break;
            default:
                Break = TRUE;
                break;
            }
        }
    }

    return HasEaten;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: g�re le mouvement d'une pi�ce dame.
 Elle permet � la dame de se d�placer en diagonale dans toutes les directions.
 Elle utilise une boucle pour v�rifier les cases dans chaque direction et
 ajoute les cases vides � l'arbre de positions.
 */
static void CheckerTable_QueenMovement(CheckerPieceMatrix* Table,
                                       CheckerPiece* Origin,
                                       CheckerPositionTree* Tree,
                                       CPosition Pos)
{
    int i;
    int Enemy;
    CTValidation Res;

    // Top left
    i = 1;
    Enemy = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X - i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            Enemy++;
            if (Enemy >= 2)
            {
                break;
            }
        }
        if (Res == CTV_EMPTY_PLACE)
        {
            Enemy = 0;
            CheckerPositionTree_Insert(Tree,
                                       CPT_TOP_LEFT, Pos.X - i, Pos.Y - i);
        }
        i++;
    }

    // Top right
    i = 1;
    Enemy = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X + i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            Enemy++;
            if (Enemy >= 2)
            {
                break;
            }
        }
        if (Res == CTV_EMPTY_PLACE)
        {
            Enemy = 0;
            CheckerPositionTree_Insert(Tree,
                                       CPT_TOP_RIGHT, Pos.X + i, Pos.Y - i);
        }
        i++;
    }

    // Bottom left
    i = 1;
    Enemy = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X - i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            Enemy++;
            if (Enemy >= 2)
            {
                break;
            }
        }
        if (Res == CTV_EMPTY_PLACE)
        {
            Enemy = 0;
            CheckerPositionTree_Insert(Tree,
                                       CPT_BOTTOM_LEFT, Pos.X - i, Pos.Y + i);
        }
        i++;
    }

    // Bottom right
    i = 1;
    Enemy = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X + i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            Enemy++;
            if (Enemy >= 2)
            {
                break;
            }
        }
        if (Res == CTV_EMPTY_PLACE)
        {
            Enemy = 0;
            CheckerPositionTree_Insert(Tree,
                                       CPT_BOTTOM_RIGHT, Pos.X + i, Pos.Y + i);
        }
        i++;
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: g�re la possibilit� de manger une pi�ce avec une dame.
 Elle v�rifie les cases dans chaque direction en utilisant une boucle et
 ajoute les cases vides � l'arbre de positions.
 */
static gboolean CheckerTable_QueenEat(CheckerPieceMatrix* Table,
                                      CheckerPiece* Origin, CheckerPositionTree* Tree, CPosition Pos)
{
    int i;
    CTValidation Res;

    uint32_t EatStep = 0;
    uint32_t InsertCount = 0;

    // Top left
    i = 1;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X - i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            i++;
            while ((Res = CheckerTable_Validate(Table, Origin, Pos.X - i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
            {
                if (Res == CTV_EMPTY_PLACE)
                {
                    if (Tree)
                    {
                        InsertCount++;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_TOP_LEFT, Pos.X - i, Pos.Y - i);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    break;
                }
                i++;
            }
            break;
        }
        i++;
    }

    // Top right
    i = 1;
    EatStep = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X + i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            i++;
            while ((Res = CheckerTable_Validate(Table,
                                                Origin, Pos.X + i, Pos.Y - i)) != CTV_OUT_OF_BOUNDS)
            {
                if (Res == CTV_EMPTY_PLACE)
                {
                    if (Tree)
                    {
                        InsertCount++;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_TOP_RIGHT, Pos.X + i, Pos.Y - i);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    break;
                }
                i++;
            }
            break;
        }
        i++;
    }

    // Bottom left
    i = 1;
    EatStep = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X - i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            i++;
            while ((Res = CheckerTable_Validate(Table,
                                                Origin, Pos.X - i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
            {
                if (Res == CTV_EMPTY_PLACE)
                {
                    if (Tree)
                    {
                        InsertCount++;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_BOTTOM_LEFT, Pos.X - i, Pos.Y + i);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    break;
                }
                i++;
            }
            break;
        }
        i++;
    }

    // Bottom right
    i = 1;
    EatStep = 0;
    while ((Res = CheckerTable_Validate(Table,
                                        Origin, Pos.X + i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
    {
        if (Res == CTV_SAME_TEAM)
        {
            break;
        }
        if (Res == CTV_DIFFERENT_TEAM)
        {
            i++;
            while ((Res = CheckerTable_Validate(Table,
                                                Origin, Pos.X + i, Pos.Y + i)) != CTV_OUT_OF_BOUNDS)
            {
                if (Res == CTV_EMPTY_PLACE)
                {
                    if (Tree)
                    {
                        InsertCount++;
                        CheckerPositionTree_Insert(Tree,
                                                   CPT_BOTTOM_RIGHT, Pos.X + i, Pos.Y + i);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                else
                {
                    break;
                }
                i++;
            }
            break;
        }
        i++;
    }
    return InsertCount > 0;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:Cette fonction v�rifie si une pi�ce peut effectuer une capture.
 Elle appelle soit CheckerTable_QueenEat (pour les dames)
 ou CheckerTable_BasicEat (pour les pions) en fonction du type de pi�ce.
 */
static gboolean CheckerTable_Eat(CheckerPieceMatrix* Table,
                                 CheckerPiece* Origin,
                                 CheckerPositionTree* Tree, CPosition Pos)
{
    return Origin->IsQueen ? CheckerTable_QueenEat(Table, Origin, Tree, Pos) : CheckerTable_BasicEat(Table, Origin, Tree, Pos);
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:Cette fonction g�re le d�placement d'une pi�ce.
 Si la pi�ce est une dame, elle appelle CheckerTable_QueenMovement,
 sinon elle appelle CheckerTable_BasicMovement.
 */
static void CheckerTable_Movement(CheckerPieceMatrix* Table,
                                  CheckerPiece* Origin, CheckerPositionTree* Tree,
                                  CPosition Pos)
{
    if (Origin->IsQueen)
    {
        // Si la pi�ce est une dame, appelle la fonction CheckerTable_QueenMovement
        CheckerTable_QueenMovement(Table, Origin, Tree, Pos);
    }
    else
    {
        // Si la pi�ce n'est pas une dame, appelle la fonction CheckerTable_BasicMovement
        CheckerTable_BasicMovement(Table, Origin, Tree, Pos);
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:Cette fonction ins�re une position
 dans l'arbre des positions de jeu.
 La position est ajout�e � l'une des quatre parties de
 l'arbre en fonction de la direction sp�cifi�e.
 */
void CheckerPositionTree_Insert(CheckerPositionTree* Tree, CPTPosition Position, char X, char Y)
{
    CPosition* Tmp; // Pointeur temporaire pour la r�allocation de m�moire

    switch (Position)
    {
    case CPT_TOP_LEFT:
    {
        // R�alloue la m�moire pour agrandir le tableau Tree->TopLeft
        Tmp = (CPosition*)realloc(Tree->TopLeft, sizeof(CPosition) * (Tree->TopLeftCount + 1));
        // V�rifie si la r�allocation a r�ussi
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert");
        // Affecte la valeur X � la nouvelle position dans le tableau
        Tmp[Tree->TopLeftCount].X = X;
        // Affecte la valeur Y � la nouvelle position dans le tableau, puis incr�mente TopLeftCount
        Tmp[Tree->TopLeftCount++].Y = Y;
        // Met � jour le pointeur Tree->TopLeft pour qu'il pointe vers la nouvelle m�moire allou�e
        Tree->TopLeft = Tmp;
        break;
    }
    case CPT_TOP_RIGHT:
    {
        // R�alloue la m�moire pour agrandir le tableau Tree->TopRight
        Tmp = (CPosition*)realloc(Tree->TopRight, sizeof(CPosition) * (Tree->TopRightCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // V�rifie si la r�allocation a r�ussi
        Tmp[Tree->TopRightCount].X = X;                  // Affecte la valeur X � la nouvelle position dans le tableau
        Tmp[Tree->TopRightCount++].Y = Y;                // Affecte la valeur Y � la nouvelle position dans le tableau, puis incr�mente TopRightCount
        Tree->TopRight = Tmp;                            // Met � jour le pointeur Tree->TopRight pour qu'il pointe vers la nouvelle m�moire allou�e
        break;
    }
    case CPT_BOTTOM_LEFT:
    {
        // R�alloue la m�moire pour agrandir le tableau Tree->BottomLeft
        Tmp = (CPosition*)realloc(Tree->BottomLeft, sizeof(CPosition) * (Tree->BottomLeftCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // V�rifie si la r�allocation a r�ussi
        Tmp[Tree->BottomLeftCount].X = X;                // Affecte la valeur X � la nouvelle position dans le tableau
        Tmp[Tree->BottomLeftCount++].Y = Y;              // Affecte la valeur Y � la nouvelle position dans le tableau, puis incr�mente BottomLeftCount
        Tree->BottomLeft = Tmp;                          // Met � jour le pointeur Tree->BottomLeft pour qu'il pointe vers la nouvelle m�moire allou�e
        break;
    }
    case CPT_BOTTOM_RIGHT:
    {
        // R�alloue la m�moire pour agrandir le tableau Tree->BottomRight
        Tmp = (CPosition*)realloc(Tree->BottomRight, sizeof(CPosition) * (Tree->BottomRightCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // V�rifie si la r�allocation a r�ussi
        Tmp[Tree->BottomRightCount].X = X;               // Affecte la valeur X � la nouvelle position dans le tableau
        Tmp[Tree->BottomRightCount++].Y = Y;             // Affecte la valeur Y � la nouvelle position dans le tableau, puis incr�mente BottomRightCount
        Tree->BottomRight = Tmp;                         // Met � jour le pointeur Tree->BottomRight pour qu'il pointe vers la nouvelle m�moire allou�e
        break;
    }
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:Cette fonction v�rifie si une pi�ce peut se d�placer.
 Si la pi�ce ne peut pas effectuer de capture,
 elle examine si d'autres pi�ces de la m�me �quipe peuvent effectuer une capture.
 Elle renvoie une valeur bool�enne indiquant si la pi�ce peut se d�placer.
 */
static int CheckerTable_PieceCanMove(CheckerPieceInfo* Table, CPosition Pos, CheckerPiece* Origin)
{
    // Si on ne peut pas manger, on peut bouger
    if (!CheckerTable_Eat(&Table->Pieces, Origin, NULL, Pos))
    {
        CheckerPieceRel* Rel = Origin->Team == CPT_White ? Table->WhitePieces : Table->BlackPieces;
        // Si quelqu'un peut manger, il doit manger
        for (int i = 0; i < CHECKER_PIECE_COUNT; i++, Rel++)
        {
            if (CheckerPieceRel_Exists(Rel))
            {
                // V�rifie si la position de la pi�ce courante est diff�rente de la position de destination
                if (Rel->Pos.X != Pos.X || Rel->Pos.Y != Pos.Y)
                {
                    // Met � jour l'origine avec la pi�ce courante
                    Origin = &Table->Pieces.Mat[Rel->Pos.Y][Rel->Pos.X];
                    // V�rifie si la pi�ce courante peut manger une autre pi�ce
                    if (CheckerTable_Eat(&Table->Pieces, Origin, NULL, Rel->Pos))
                    {
                        return FALSE; // Retourne FALSE si une pi�ce peut manger une autre pi�ce
                    }
                }
            }
        }
    }
    return TRUE; // Retourne TRUE si la pi�ce peut bouger sans manger
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction r�cup�re tous les mouvements
 possibles pour une pi�ce donn�e � une position donn�e.
 Elle utilise les fonctions CheckerTable_Eat et
 CheckerTable_Movement pour d�terminer les mouvements possibles.
 */
void CheckerTable_GetFutures(CheckerPieceInfo* Table, CPosition Pos, CheckerPositionTree* Tree, gboolean OnlyEat)
{
    // Obtient la pi�ce d'origine � partir de la position sp�cifi�e
    CheckerPiece* Origin = &Table->Pieces.Mat[Pos.Y][Pos.X];

    if (CheckerTable_PieceCanMove(Table, Pos, Origin))
    {
        // Si la pi�ce peut bouger sans manger
        if (!CheckerTable_Eat(&Table->Pieces, Origin, Tree, Pos))
        {
            if (!OnlyEat)
            {
                // Si la pi�ce ne peut pas manger d'autres pi�ces, g�n�re les mouvements possibles
                CheckerTable_Movement(&Table->Pieces, Origin, Tree, Pos);
            }
        }
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction lib�re la m�moire utilis�e
 par l'arbre des positions de jeu.
 */
void CheckerPositionTree_Free(CheckerPositionTree* Tree)
{
    // Lib�re la m�moire des tableaux s'ils existent
    if (Tree->TopLeft)
        free(Tree->TopLeft); // Lib�re la m�moire du tableau TopLeft
    if (Tree->TopRight)
        free(Tree->TopRight); // Lib�re la m�moire du tableau TopRight
    if (Tree->BottomLeft)
        free(Tree->BottomLeft); // Lib�re la m�moire du tableau BottomLeft
    if (Tree->BottomRight)
        free(Tree->BottomRight); // Lib�re la m�moire du tableau BottomRight

    // R�initialise les pointeurs des tableaux � NULL
    Tree->TopLeft = NULL;
    Tree->TopRight = NULL;
    Tree->BottomLeft = NULL;
    Tree->BottomRight = NULL;

    // R�initialise les compteurs des tableaux � 0
    Tree->BottomLeftCount = 0;
    Tree->BottomRightCount = 0;
    Tree->TopLeftCount = 0;
    Tree->TopRightCount = 0;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction v�rifie si une relation de pi�ce
 existe (si la position n'est pas �gale � -1).
 */
gboolean CheckerPieceRel_Exists(CheckerPieceRel* Rel)
{
    return Rel->Pos.X != -1;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction met fin � la partie et affiche
 le message de victoire correspondant � l'�quipe sp�cifi�e.
 */
void CheckerTable_EndGame(CheckerTable* Table, CheckerPieceTeam Team)
{
    // V�rifie l'�quipe gagnante et met � jour le titre de la fen�tre en cons�quence
    if (Team == CPT_White)
    {
        // Met � jour le titre de la fen�tre pour indiquer que les blancs ont gagn�
        gtk_window_set_title(GTK_WINDOW(Table->Window), "Les blancs ont gagne!");
    }
    else
    {
        // Met � jour le titre de la fen�tre pour indiquer que les noirs ont gagn�
        gtk_window_set_title(GTK_WINDOW(Table->Window), "Les noirs ont gagne!");
    }
    Table->Turn = CPT_Empty;
}

//

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction met � jour l'affichage
 du tableau de jeu en fonction de l'�tat actuel des pi�ces.
 */
void CheckerTable_Refresh(CheckerTable* Table)
{
    // Masque les images des pi�ces blanches actives
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        if (Table->Active.WhitePieces[i].Image)
            gtk_widget_hide(Table->Active.WhitePieces[i].Image); // Masque l'image de la pi�ce blanche si elle existe
        if (Table->Active.BlackPieces[i].Image)
            gtk_widget_hide(Table->Active.BlackPieces[i].Image); // Masque l'image de la pi�ce noire si elle existe
    }

    int AbsX, AbsY;
    CheckerPieceRel* Rel;

    int White = 0, Black = 0;

    // Actualise les positions des pi�ces blanches actives et les affiche
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        Rel = &Table->Active.WhitePieces[i];
        if (CheckerPieceRel_Exists(Rel))
        {
            White++;
            CheckerTable_QueenUpdate(Table, Rel);                  // Met � jour le statut de dame de la pi�ce blanche
            CheckerTable_AbsolutePosition(Rel->Pos, &AbsX, &AbsY); // Obtient la position absolue de la pi�ce
            gtk_fixed_move(Table->Fixed, Rel->Image, AbsX, AbsY);  // D�place l'image de la pi�ce � la position absolue
            gtk_widget_show(Rel->Image);                           // Affiche l'image de la pi�ce blanche
        }

        // Actualise les positions des pi�ces noires actives et les affiche
        Rel = &Table->Active.BlackPieces[i];
        if (CheckerPieceRel_Exists(Rel))
        {
            Black++;
            CheckerTable_QueenUpdate(Table, Rel);                  // Met � jour le statut de dame de la pi�ce noire
            CheckerTable_AbsolutePosition(Rel->Pos, &AbsX, &AbsY); // Obtient la position absolue de la pi�ce
            gtk_fixed_move(Table->Fixed, Rel->Image, AbsX, AbsY);  // D�place l'image de la pi�ce � la position absolue
            gtk_widget_show(Rel->Image);                           // Affiche l'image de la pi�ce noire
        }
    }

    White = CHECKER_PIECE_COUNT - White;
    Black = CHECKER_PIECE_COUNT - Black;

    char Text[30];
    sprintf(Text, "Score ( Red: %d -- Black: %d )", Black, White);
    gtk_label_set_text(Table->ScoreLabel, Text);
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction d�finit le tour de jeu pour une �quipe
 sp�cifi�e et met � jour le titre de la fen�tre.
 */
void CheckerTable_SetTurn(CheckerTable* Table, CheckerPieceTeam Team)
{
    static char Text[64];
    Text[0] = '\0'; // Initialise la cha�ne de caract�res Text avec une cha�ne vide

    switch (Table->Mode)
    {
    case CPM_PlayerVsPlayer:
    case CPM_PlayerVsRPC:
    {
        // Ajoute la couleur de l'�quipe actuelle � la cha�ne Text
        sprintf(Text, "(%s) ", Table->Team == CPT_White ? "Red" : "Black");
        break;
    }
    }

    // Ajoute le texte indiquant de qui est le tour � la cha�ne Text
    sprintf(Text, "%s%s's Turn", Text, Team == CPT_White ? "Red" : "Black");

    Table->Turn = Team;                        // D�finit l'�quipe dont c'est le tour
    gtk_window_set_title(Table->Window, Text); // Met � jour le titre de la fen�tre avec le texte indiquant le tour actuel
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction d�finit le tour de jeu
 pour l'�quipe suivante en fonction de l'�quipe actuelle.
 */
void CheckerTable_SetNextTurn(CheckerTable* Table)
{
    CheckerTable_SetTurn(Table, Table->Turn == CPT_Black ? CPT_White : CPT_Black);
}

//

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction enregistre un instantan� de l'�tat actuel
 du tableau de jeu en tant que historique pour une utilisation
 ult�rieure lors de l'annulation d'un mouvement.
 */
void CheckerTable_Snapshot(CheckerTable* Table)
{
    CheckerPieceHistory* Tmp;

    // Supprime les anciens historiques de pi�ces
    while (Table->Old)
    {
        Tmp = Table->Old;
        Table->Old = Tmp->Next;
        free(Tmp);
    }

    // Cr�e un nouvel historique de pi�ces
    Tmp = (CheckerPieceHistory*)calloc(1, sizeof(CheckerPieceHistory));
    Malloc_Valid(Tmp, "CheckerTable_Undo");
    Tmp->Table = Table->Final;

    if (Table->Snapshot)
    {
        Tmp->Next = Table->Snapshot;
    }
    Table->Snapshot = Tmp;
}

//

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction affiche l'�tat
 actuel du tableau de jeu sur la console.
 */
void CheckerTable_Print(CheckerPieceMatrix* Pieces)
{
    for (int i = 0; i < CHECKER_TABLE_X; i++)
    {
        printf("  "); // Affiche un espace avant chaque ligne pour l'alignement

        for (int j = 0; j < CHECKER_TABLE_Y; j++)
        {
            printf("%d ", Pieces->Mat[i][j].Team); // Affiche l'identifiant de l'�quipe de chaque pi�ce
        }

        printf("\n"); // Passe � la ligne suivante
    }

    printf("\n"); // Ajoute une ligne vide � la fin pour la lisibilit�
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction passe au tour suivant en
 r�initialisant certains param�tres du jeu.
 */
void CheckerTable_NextTurn(CheckerTable* Table)
{
    Table->NeedNetUpdate = TRUE; // D�finit le drapeau indiquant qu'une mise � jour du r�seau est n�cessaire

    Table->ActivePiece = NULL; // R�initialise la pi�ce active (aucune pi�ce active)
    Table->NumberOfTurns = 0;  // R�initialise le nombre de tours

    CheckerTable_Refresh(Table);       // Actualise l'affichage du plateau
    CheckerTable_DestroyFuture(Table); // D�truit les futurs mouvements possibles

    CheckerTable_SetNextTurn(Table); // D�finit le prochain tour
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:  Cette fonction transf�re un �l�ment de
 l'historique des pi�ces d'une liste � une autre liste
 */
CheckerPieceHistory* CPH_Transfer(CheckerPieceHistory** From,
                                  CheckerPieceHistory** To)
{
    CheckerPieceHistory* Tmp = *From;
    *From = Tmp->Next;
    Tmp->Next = *To;
    *To = Tmp;
    return Tmp;
}

/*
 * Entr�e:
 * Sortie:
 * Fonction: Cette fonction annule le dernier mouvement
 effectu� en restaurant l'�tat pr�c�dent du tableau de jeu.
 */
void CheckerTable_Undo(CheckerTable* Table)
{
    if (Table->Snapshot)
    {
        CheckerPieceHistory* Tmp = CPH_Transfer(&Table->Snapshot, &Table->Old); // Transf�re l'�l�ment le plus r�cent de l'historique des snapshots vers l'historique des anciens �tats

        Table->Final = Table->Old->Table;  // Met � jour l'�tat final avec l'�tat de l'ancien �l�ment
        Table->Active = Table->Old->Table; // Met � jour l'�tat actif avec l'�tat de l'ancien �l�ment

        // CheckerTable_Print(&Table->Final.Pieces); // Affiche l'�tat final (debug)
        CheckerTable_NextTurn(Table); // Passe au prochain tour
    }
}

/*
 * Entr�e:
 * Sortie:
 * Fonction:  Cette fonction r�tablit le dernier
 mouvement annul� en restaurant l'�tat suivant du tableau de jeu.
 */
void CheckerTable_Redo(CheckerTable* Table)
{
    if (Table->Old)
    {
        CheckerPieceHistory* Tmp = CPH_Transfer(&Table->Old, &Table->Snapshot); // Transf�re l'�l�ment le plus r�cent de l'historique des anciens �tats vers l'historique des snapshots

        Table->Final = Tmp->Table;  // Met � jour l'�tat final avec l'�tat de l'�l�ment transf�r�
        Table->Active = Tmp->Table; // Met � jour l'�tat actif avec l'�tat de l'�l�ment transf�r�

        // CheckerTable_Print(&Table->Final.Pieces); // Affiche l'�tat final (debug)
        CheckerTable_NextTurn(Table); // Passe au prochain tour
    }
}

gboolean CheckerTable_TryEndGame(CheckerTable* Table)
{

    // verifier si le jeu est termine
    uint32_t WhiteCount = 0, BlackCount = 0;
    // parcours de toutes les pieces blanc
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        if (CheckerPieceRel_Exists(&Table->Final.WhitePieces[i]))
        {
            WhiteCount++;
            break;
        }
    }
    // parcours de toutes les pieces noir
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        if (CheckerPieceRel_Exists(&Table->Final.BlackPieces[i]))
        {
            BlackCount++;
            break;
        }
    } // fin de for

    // s'il n'y a pas des pions de noir ou blanc dans la table
    if (!WhiteCount || !BlackCount)
    {
        // preparer la fin du jeu
        CheckerTable_EndGame(Table, !WhiteCount ? CPT_Black : CPT_White);
        return TRUE;
    } // fin de for

    return FALSE;
}
