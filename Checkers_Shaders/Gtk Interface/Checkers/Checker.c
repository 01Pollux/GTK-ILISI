#include <gdk/gdkkeysyms.h>
#include "Checker.h"
#include "Bot.h"
#include <Windows.h>
#include <mmsystem.h>

/*
* Entrée: La table de jeu et la nouvelle position
* Sortie: Un boolean
* Fonction: Cette fonction vérifie si une position donnée
peut être traversée dans le future.
Elle parcourt les différentes positions possibles
dans les différentes directions
(en haut à gauche, en haut à droite,
en bas à gauche et en bas à droite)
et vérifie si la position donnée correspond à l'une de ces positions.
*/
static gboolean CheckerTable_CanTraverseCurrentFuture(CheckerTable* Table, CPosition NewPos)
{
    // Pour tout les pieces actives haut gauche
    for (int i = 0; i < Table->ActivePieceFutures.TopLeftCount; i++)
    {
        // On verifie si les deux cordonnées nouvelles des pieces est
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
* Fonction: Cette fonction déplace une image future
(représentant un mouvement potentiel) sur une table de jeu.
Elle utilise les coordonnées absolues
fournies pour déplacer l'image future sur le tableau.
*/
static void CheckerTable_MoveFutureImage(CheckerTable* Table,
                                         CPosition Pos)
{
    int AbsX, AbsY;
    // Recuperation de la position absolue
    CheckerTable_AbsolutePosition(Pos, &AbsX, &AbsY);
    // Changement de l'image á l'image future
    gtk_fixed_move(Table->Fixed, Table->FuturePiece, AbsX, AbsY);
}

/*
* Entrée:
* Sortie:
* Fonction: Cette fonction détruit
une image future et cache le widget associé.
*/
void CheckerTable_DestroyFuture(CheckerTable* Table)
{
    CheckerPositionTree_Free(&Table->ActivePieceFutures);
    gtk_widget_hide(Table->FuturePiece);
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction vérifie si une
 pièce peut manger une autre pièce
 dans le chemin entre deux positions données.
 Elle utilise les coordonnées de départ et d'arrivée
 pour déterminer la direction du mouvement.
 Elle parcourt ensuite les positions intermédiaires
 et vérifie si une pièce peut être mangée.
 Elle renvoie un score basé sur
 le nombre de pièces mangées et le type de pièces.
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

    // Tant que la position n'est pas la position d'arrivée
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
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction effectue un déplacement
 d'une pièce d'une position donnée à une position donnée.
 Elle utilise CheckerTable_EatInPath pour vérifier
 si des pièces doivent être mangées dans le chemin du déplacement.
 Elle met à jour les informations de la pièce et retourne un score
 basé sur les pièces mangées et les changements de statut
 (par exemple, une pièce devient une reine).
 */
int CheckerTable_MovePiece(CheckerPieceInfo* Info,
                           CheckerPieceTeam Team,
                           CPosition From, CPosition To)
{
    // On récupère le score
    int Score = CheckerTable_EatInPath(Info, Team, From, To);

    // Déplace la pièce de la position From à la position To dans la matrice des pièces
    Info->Pieces.Mat[To.Y][To.X] = Info->Pieces.Mat[From.Y][From.X];
    Info->Pieces.Mat[From.Y][From.X].Team = CPT_Empty;

    // Récupère un pointeur vers la pièce déplacée
    CheckerPiece* Piece = &Info->Pieces.Mat[To.Y][To.X];
    CheckerPieceRel* Rel;

    // Vérifie si la pièce était une reine avant le déplacement
    gboolean WasQueen = Piece->IsQueen;

    // Met à jour la position de la pièce relative à son équipe (blanche ou noire)
    if (Piece->Team == CPT_White)
    {
        Rel = &Info->WhitePieces[Piece->Index];
        // Si la pièce atteint la rangée 0, elle devient une reine
        if (To.Y == 0)
        {
            Piece->IsQueen = TRUE;
        }
    }
    else
    {
        Rel = &Info->BlackPieces[Piece->Index];
        // Si la pièce atteint la rangée 7, elle devient une reine
        if (To.Y == 7)
        {
            Piece->IsQueen = TRUE;
        }
    }

    // Si le statut de reine de la pièce a changé, ajoute 4 au score
    if (Piece->IsQueen != WasQueen)
    {
        Score += 4;
    }

    // Met à jour la position relative de la pièce
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
 * Entrée: Fenetre GTK, Evenement, la table de jeu
 * Sortie: boolean
 * Fonction: Cette fonction est appelée
 lorsqu'un bouton de la table est pressé.
 Elle gère les interactions du joueur avec la table.
 Selon le mode de jeu et l'état actuel de la table,
 elle effectue différentes actions, comme la sélection d'une pièce,
 le déplacement d'une pièce ou le changement de tour.
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
        // Si une pièce est déjà active, on traite le déplacement
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
            // Si la pièce appartient à l'adversaire ou si ce n'est pas le premier mouvement du joueur actif, on ignore l'action
            return TRUE;
        }
        // Traitement du changement de pièce active
    }

    if (CheckerTable_TrySelect(Table, NewPos))
    {
        // Si la sélection d'une nouvelle pièce est réussie, on détruit les mouvements futurs et on récupère les futures positions possibles pour la nouvelle pièce active
        CheckerTable_DestroyFuture(Table);
        CheckerTable_GetFutures(&Table->Active, Table->ActivePiece->Pos, &Table->ActivePieceFutures, FALSE);

        CheckerTable_MoveFutureImage(Table, NewPos);
        gtk_widget_show(Table->FuturePiece);
    }

    return TRUE;
}

// CHANGING TURNS OR RESET/REWIND
/*
 * Entrée:Evenement et table
 * Sortie: boolean
 * Fonction:Cette fonction est appelée lorsqu'une touche est
 pressée sur la table.
 Elle gère les actions spécifiques liées
 à la touche pressée,
 comme le passage au tour suivant ou
 la réinitialisation/retour en arrière.
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
 * Entrée:
 * Sortie:
 * Fonction:Cette fonction est appelée lorsqu'un mouvement de souris
 est détecté sur la table.
 Elle gère le déplacement d'une pièce future en fonction de la
 position de la souris.
 Elle vérifie également si une pièce peut être traversée
 dans le futur à la position actuelle de la souris.
 */
gboolean CheckerTable_OnMouseMove(GtkWidget* Widget, GdkEventMotion* Event, CheckerTable* Table)
{
    // Vérifier si aucune pièce active n'est sélectionnée
    if (!Table->ActivePiece)
        return FALSE;

    CPosition Pos;
    // Calculer la position relative de la souris par rapport à la table de jeu
    CheckerTable_RelativePosition((int)Event->x - 50, (int)Event->y - 100, &Pos);

    // Vérifier si la pièce active peut traverser vers la position actuelle de la souris
    gboolean ShouldDisplay = CheckerTable_CanTraverseCurrentFuture(Table, Pos);

    // Si la pièce peut traverser vers cette position, mettre à jour l'affichage de la future position
    if (ShouldDisplay)
    {
        CheckerTable_MoveFutureImage(Table, Pos);
    }

    return FALSE;
}

//
/*
 * Entrée:Positions
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
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction tente de sélectionner
 une pièce à partir de la position donnée.
 Si une pièce du joueur correspondant est présente à cette position,
 elle est sélectionnée et une valeur booléenne
 est renvoyée pour indiquer le succès de la sélection.
 */
gboolean CheckerTable_TrySelect(CheckerTable* Table, CPosition Pos)
{
    // Récupérer la pièce située à la position donnée
    CheckerPiece* Piece = &Table->Active.Pieces.Mat[Pos.Y][Pos.X];

    // Vérifier si la pièce appartient à l'équipe en cours de jeu
    if (Piece->Team == Table->Turn)
    {
        // Sélectionner la pièce active en fonction de l'équipe en cours
        if (Table->Turn == CPT_White)
        {
            Table->ActivePiece = &Table->Active.WhitePieces[Piece->Index];
        }
        else
        {
            Table->ActivePiece = &Table->Active.BlackPieces[Piece->Index];
        }

        // Vérifier si la pièce active existe dans la structure CheckerPieceRel
        if (CheckerPieceRel_Exists(Table->ActivePiece))
        {
            return TRUE; // La sélection de la pièce est réussie
        }
    }

    // Réinitialiser la pièce active et retourner FALSE pour indiquer l'échec de la sélection
    Table->ActivePiece = NULL;
    return FALSE;
}

//

/*
 * Entrée:
 * Sortie:
 * Fonction:  renvoie un pointeur vers la pièce d'origine à partir de
 la matrice des pièces, en fonction des coordonnées fournies.
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
 * Entrée:
 * Sortie:
 * Fonction: vérifie si le mouvement d'une pièce est valide en vérifiant
 les cases d'origine et de destination.
 Elle renvoie différentes valeurs
 (CTV_OUT_OF_BOUNDS, CTV_EMPTY_PLACE, CTV_DIFFERENT_TEAM, CTV_SAME_TEAM)
 pour indiquer le résultat de la validation.
 */
static CTValidation CheckerTable_Validate(CheckerPieceMatrix* Table, CheckerPiece* Origin, char DestX, char DestY)
{
    // Récupérer la pièce de destination en fonction des coordonnées fournies
    CheckerPiece* Dest = CheckerTable_GetOrigin(Table, DestX, DestY);

    // Vérifier si la pièce de destination existe
    if (!Dest)
    {
        return CTV_OUT_OF_BOUNDS; // La pièce est en dehors des limites de la table
    }

    // Vérifier si la case de destination est vide
    if (Dest->Team == CPT_Empty)
    {
        return CTV_EMPTY_PLACE; // La case de destination est vide
    }

    // Vérifier si la pièce de destination est de l'équipe différente de la pièce d'origine
    // Si c'est le cas, cela indique que la pièce d'origine peut potentiellement effectuer une capture (manger)
    return (Dest->Team != Origin->Team) ? CTV_DIFFERENT_TEAM : CTV_SAME_TEAM;
}

/*
 * Entrée:
 * Sortie:
 * Fonction: insère une position dans un arbre de positions.
 */
void CheckerPositionTree_Insert(CheckerPositionTree* Tree, CPTPosition Position, char X, char Y);

/*
 * Entrée:
 * Sortie:
 * Fonction: gère le mouvement basique d'une pièce. Selon l'équipe
 de la pièce d'origine,
 elle vérifie les mouvements possibles vers le haut gauche, le haut droit,
 le bas gauche et le bas droit.
 Si une case est vide, elle l'ajoute à l'arbre de positions.
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
 * Entrée:
 * Sortie:
 * Fonction: gère la possibilité de manger une pièce.
 Elle vérifie si une pièce peut manger une autre en regardant les cases environnantes.
 Si une pièce adverse est présente,
 elle vérifie si la case après celle-ci est vide et l'ajoute à l'arbre de positions.
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
 * Entrée:
 * Sortie:
 * Fonction: gère le mouvement d'une pièce dame.
 Elle permet à la dame de se déplacer en diagonale dans toutes les directions.
 Elle utilise une boucle pour vérifier les cases dans chaque direction et
 ajoute les cases vides à l'arbre de positions.
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
 * Entrée:
 * Sortie:
 * Fonction: gère la possibilité de manger une pièce avec une dame.
 Elle vérifie les cases dans chaque direction en utilisant une boucle et
 ajoute les cases vides à l'arbre de positions.
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
 * Entrée:
 * Sortie:
 * Fonction:Cette fonction vérifie si une pièce peut effectuer une capture.
 Elle appelle soit CheckerTable_QueenEat (pour les dames)
 ou CheckerTable_BasicEat (pour les pions) en fonction du type de pièce.
 */
static gboolean CheckerTable_Eat(CheckerPieceMatrix* Table,
                                 CheckerPiece* Origin,
                                 CheckerPositionTree* Tree, CPosition Pos)
{
    return Origin->IsQueen ? CheckerTable_QueenEat(Table, Origin, Tree, Pos) : CheckerTable_BasicEat(Table, Origin, Tree, Pos);
}

/*
 * Entrée:
 * Sortie:
 * Fonction:Cette fonction gère le déplacement d'une pièce.
 Si la pièce est une dame, elle appelle CheckerTable_QueenMovement,
 sinon elle appelle CheckerTable_BasicMovement.
 */
static void CheckerTable_Movement(CheckerPieceMatrix* Table,
                                  CheckerPiece* Origin, CheckerPositionTree* Tree,
                                  CPosition Pos)
{
    if (Origin->IsQueen)
    {
        // Si la pièce est une dame, appelle la fonction CheckerTable_QueenMovement
        CheckerTable_QueenMovement(Table, Origin, Tree, Pos);
    }
    else
    {
        // Si la pièce n'est pas une dame, appelle la fonction CheckerTable_BasicMovement
        CheckerTable_BasicMovement(Table, Origin, Tree, Pos);
    }
}

/*
 * Entrée:
 * Sortie:
 * Fonction:Cette fonction insère une position
 dans l'arbre des positions de jeu.
 La position est ajoutée à l'une des quatre parties de
 l'arbre en fonction de la direction spécifiée.
 */
void CheckerPositionTree_Insert(CheckerPositionTree* Tree, CPTPosition Position, char X, char Y)
{
    CPosition* Tmp; // Pointeur temporaire pour la réallocation de mémoire

    switch (Position)
    {
    case CPT_TOP_LEFT:
    {
        // Réalloue la mémoire pour agrandir le tableau Tree->TopLeft
        Tmp = (CPosition*)realloc(Tree->TopLeft, sizeof(CPosition) * (Tree->TopLeftCount + 1));
        // Vérifie si la réallocation a réussi
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert");
        // Affecte la valeur X à la nouvelle position dans le tableau
        Tmp[Tree->TopLeftCount].X = X;
        // Affecte la valeur Y à la nouvelle position dans le tableau, puis incrémente TopLeftCount
        Tmp[Tree->TopLeftCount++].Y = Y;
        // Met à jour le pointeur Tree->TopLeft pour qu'il pointe vers la nouvelle mémoire allouée
        Tree->TopLeft = Tmp;
        break;
    }
    case CPT_TOP_RIGHT:
    {
        // Réalloue la mémoire pour agrandir le tableau Tree->TopRight
        Tmp = (CPosition*)realloc(Tree->TopRight, sizeof(CPosition) * (Tree->TopRightCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // Vérifie si la réallocation a réussi
        Tmp[Tree->TopRightCount].X = X;                  // Affecte la valeur X à la nouvelle position dans le tableau
        Tmp[Tree->TopRightCount++].Y = Y;                // Affecte la valeur Y à la nouvelle position dans le tableau, puis incrémente TopRightCount
        Tree->TopRight = Tmp;                            // Met à jour le pointeur Tree->TopRight pour qu'il pointe vers la nouvelle mémoire allouée
        break;
    }
    case CPT_BOTTOM_LEFT:
    {
        // Réalloue la mémoire pour agrandir le tableau Tree->BottomLeft
        Tmp = (CPosition*)realloc(Tree->BottomLeft, sizeof(CPosition) * (Tree->BottomLeftCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // Vérifie si la réallocation a réussi
        Tmp[Tree->BottomLeftCount].X = X;                // Affecte la valeur X à la nouvelle position dans le tableau
        Tmp[Tree->BottomLeftCount++].Y = Y;              // Affecte la valeur Y à la nouvelle position dans le tableau, puis incrémente BottomLeftCount
        Tree->BottomLeft = Tmp;                          // Met à jour le pointeur Tree->BottomLeft pour qu'il pointe vers la nouvelle mémoire allouée
        break;
    }
    case CPT_BOTTOM_RIGHT:
    {
        // Réalloue la mémoire pour agrandir le tableau Tree->BottomRight
        Tmp = (CPosition*)realloc(Tree->BottomRight, sizeof(CPosition) * (Tree->BottomRightCount + 1));
        Malloc_Valid(Tmp, "CheckerPositionTree_Insert"); // Vérifie si la réallocation a réussi
        Tmp[Tree->BottomRightCount].X = X;               // Affecte la valeur X à la nouvelle position dans le tableau
        Tmp[Tree->BottomRightCount++].Y = Y;             // Affecte la valeur Y à la nouvelle position dans le tableau, puis incrémente BottomRightCount
        Tree->BottomRight = Tmp;                         // Met à jour le pointeur Tree->BottomRight pour qu'il pointe vers la nouvelle mémoire allouée
        break;
    }
    }
}

/*
 * Entrée:
 * Sortie:
 * Fonction:Cette fonction vérifie si une pièce peut se déplacer.
 Si la pièce ne peut pas effectuer de capture,
 elle examine si d'autres pièces de la même équipe peuvent effectuer une capture.
 Elle renvoie une valeur booléenne indiquant si la pièce peut se déplacer.
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
                // Vérifie si la position de la pièce courante est différente de la position de destination
                if (Rel->Pos.X != Pos.X || Rel->Pos.Y != Pos.Y)
                {
                    // Met à jour l'origine avec la pièce courante
                    Origin = &Table->Pieces.Mat[Rel->Pos.Y][Rel->Pos.X];
                    // Vérifie si la pièce courante peut manger une autre pièce
                    if (CheckerTable_Eat(&Table->Pieces, Origin, NULL, Rel->Pos))
                    {
                        return FALSE; // Retourne FALSE si une pièce peut manger une autre pièce
                    }
                }
            }
        }
    }
    return TRUE; // Retourne TRUE si la pièce peut bouger sans manger
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction récupère tous les mouvements
 possibles pour une pièce donnée à une position donnée.
 Elle utilise les fonctions CheckerTable_Eat et
 CheckerTable_Movement pour déterminer les mouvements possibles.
 */
void CheckerTable_GetFutures(CheckerPieceInfo* Table, CPosition Pos, CheckerPositionTree* Tree, gboolean OnlyEat)
{
    // Obtient la pièce d'origine à partir de la position spécifiée
    CheckerPiece* Origin = &Table->Pieces.Mat[Pos.Y][Pos.X];

    if (CheckerTable_PieceCanMove(Table, Pos, Origin))
    {
        // Si la pièce peut bouger sans manger
        if (!CheckerTable_Eat(&Table->Pieces, Origin, Tree, Pos))
        {
            if (!OnlyEat)
            {
                // Si la pièce ne peut pas manger d'autres pièces, génère les mouvements possibles
                CheckerTable_Movement(&Table->Pieces, Origin, Tree, Pos);
            }
        }
    }
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction libère la mémoire utilisée
 par l'arbre des positions de jeu.
 */
void CheckerPositionTree_Free(CheckerPositionTree* Tree)
{
    // Libère la mémoire des tableaux s'ils existent
    if (Tree->TopLeft)
        free(Tree->TopLeft); // Libère la mémoire du tableau TopLeft
    if (Tree->TopRight)
        free(Tree->TopRight); // Libère la mémoire du tableau TopRight
    if (Tree->BottomLeft)
        free(Tree->BottomLeft); // Libère la mémoire du tableau BottomLeft
    if (Tree->BottomRight)
        free(Tree->BottomRight); // Libère la mémoire du tableau BottomRight

    // Réinitialise les pointeurs des tableaux à NULL
    Tree->TopLeft = NULL;
    Tree->TopRight = NULL;
    Tree->BottomLeft = NULL;
    Tree->BottomRight = NULL;

    // Réinitialise les compteurs des tableaux à 0
    Tree->BottomLeftCount = 0;
    Tree->BottomRightCount = 0;
    Tree->TopLeftCount = 0;
    Tree->TopRightCount = 0;
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction vérifie si une relation de pièce
 existe (si la position n'est pas égale à -1).
 */
gboolean CheckerPieceRel_Exists(CheckerPieceRel* Rel)
{
    return Rel->Pos.X != -1;
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction met fin à la partie et affiche
 le message de victoire correspondant à l'équipe spécifiée.
 */
void CheckerTable_EndGame(CheckerTable* Table, CheckerPieceTeam Team)
{
    // Vérifie l'équipe gagnante et met à jour le titre de la fenêtre en conséquence
    if (Team == CPT_White)
    {
        // Met à jour le titre de la fenêtre pour indiquer que les blancs ont gagné
        gtk_window_set_title(GTK_WINDOW(Table->Window), "Les blancs ont gagne!");
    }
    else
    {
        // Met à jour le titre de la fenêtre pour indiquer que les noirs ont gagné
        gtk_window_set_title(GTK_WINDOW(Table->Window), "Les noirs ont gagne!");
    }
    Table->Turn = CPT_Empty;
}

//

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction met à jour l'affichage
 du tableau de jeu en fonction de l'état actuel des pièces.
 */
void CheckerTable_Refresh(CheckerTable* Table)
{
    // Masque les images des pièces blanches actives
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        if (Table->Active.WhitePieces[i].Image)
            gtk_widget_hide(Table->Active.WhitePieces[i].Image); // Masque l'image de la pièce blanche si elle existe
        if (Table->Active.BlackPieces[i].Image)
            gtk_widget_hide(Table->Active.BlackPieces[i].Image); // Masque l'image de la pièce noire si elle existe
    }

    int AbsX, AbsY;
    CheckerPieceRel* Rel;

    int White = 0, Black = 0;

    // Actualise les positions des pièces blanches actives et les affiche
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        Rel = &Table->Active.WhitePieces[i];
        if (CheckerPieceRel_Exists(Rel))
        {
            White++;
            CheckerTable_QueenUpdate(Table, Rel);                  // Met à jour le statut de dame de la pièce blanche
            CheckerTable_AbsolutePosition(Rel->Pos, &AbsX, &AbsY); // Obtient la position absolue de la pièce
            gtk_fixed_move(Table->Fixed, Rel->Image, AbsX, AbsY);  // Déplace l'image de la pièce à la position absolue
            gtk_widget_show(Rel->Image);                           // Affiche l'image de la pièce blanche
        }

        // Actualise les positions des pièces noires actives et les affiche
        Rel = &Table->Active.BlackPieces[i];
        if (CheckerPieceRel_Exists(Rel))
        {
            Black++;
            CheckerTable_QueenUpdate(Table, Rel);                  // Met à jour le statut de dame de la pièce noire
            CheckerTable_AbsolutePosition(Rel->Pos, &AbsX, &AbsY); // Obtient la position absolue de la pièce
            gtk_fixed_move(Table->Fixed, Rel->Image, AbsX, AbsY);  // Déplace l'image de la pièce à la position absolue
            gtk_widget_show(Rel->Image);                           // Affiche l'image de la pièce noire
        }
    }

    White = CHECKER_PIECE_COUNT - White;
    Black = CHECKER_PIECE_COUNT - Black;

    char Text[30];
    sprintf(Text, "Score ( Red: %d -- Black: %d )", Black, White);
    gtk_label_set_text(Table->ScoreLabel, Text);
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction définit le tour de jeu pour une équipe
 spécifiée et met à jour le titre de la fenêtre.
 */
void CheckerTable_SetTurn(CheckerTable* Table, CheckerPieceTeam Team)
{
    static char Text[64];
    Text[0] = '\0'; // Initialise la chaîne de caractères Text avec une chaîne vide

    switch (Table->Mode)
    {
    case CPM_PlayerVsPlayer:
    case CPM_PlayerVsRPC:
    {
        // Ajoute la couleur de l'équipe actuelle à la chaîne Text
        sprintf(Text, "(%s) ", Table->Team == CPT_White ? "Red" : "Black");
        break;
    }
    }

    // Ajoute le texte indiquant de qui est le tour à la chaîne Text
    sprintf(Text, "%s%s's Turn", Text, Team == CPT_White ? "Red" : "Black");

    Table->Turn = Team;                        // Définit l'équipe dont c'est le tour
    gtk_window_set_title(Table->Window, Text); // Met à jour le titre de la fenêtre avec le texte indiquant le tour actuel
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction définit le tour de jeu
 pour l'équipe suivante en fonction de l'équipe actuelle.
 */
void CheckerTable_SetNextTurn(CheckerTable* Table)
{
    CheckerTable_SetTurn(Table, Table->Turn == CPT_Black ? CPT_White : CPT_Black);
}

//

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction enregistre un instantané de l'état actuel
 du tableau de jeu en tant que historique pour une utilisation
 ultérieure lors de l'annulation d'un mouvement.
 */
void CheckerTable_Snapshot(CheckerTable* Table)
{
    CheckerPieceHistory* Tmp;

    // Supprime les anciens historiques de pièces
    while (Table->Old)
    {
        Tmp = Table->Old;
        Table->Old = Tmp->Next;
        free(Tmp);
    }

    // Crée un nouvel historique de pièces
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
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction affiche l'état
 actuel du tableau de jeu sur la console.
 */
void CheckerTable_Print(CheckerPieceMatrix* Pieces)
{
    for (int i = 0; i < CHECKER_TABLE_X; i++)
    {
        printf("  "); // Affiche un espace avant chaque ligne pour l'alignement

        for (int j = 0; j < CHECKER_TABLE_Y; j++)
        {
            printf("%d ", Pieces->Mat[i][j].Team); // Affiche l'identifiant de l'équipe de chaque pièce
        }

        printf("\n"); // Passe à la ligne suivante
    }

    printf("\n"); // Ajoute une ligne vide à la fin pour la lisibilité
}

/*
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction passe au tour suivant en
 réinitialisant certains paramètres du jeu.
 */
void CheckerTable_NextTurn(CheckerTable* Table)
{
    Table->NeedNetUpdate = TRUE; // Définit le drapeau indiquant qu'une mise à jour du réseau est nécessaire

    Table->ActivePiece = NULL; // Réinitialise la pièce active (aucune pièce active)
    Table->NumberOfTurns = 0;  // Réinitialise le nombre de tours

    CheckerTable_Refresh(Table);       // Actualise l'affichage du plateau
    CheckerTable_DestroyFuture(Table); // Détruit les futurs mouvements possibles

    CheckerTable_SetNextTurn(Table); // Définit le prochain tour
}

/*
 * Entrée:
 * Sortie:
 * Fonction:  Cette fonction transfère un élément de
 l'historique des pièces d'une liste à une autre liste
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
 * Entrée:
 * Sortie:
 * Fonction: Cette fonction annule le dernier mouvement
 effectué en restaurant l'état précédent du tableau de jeu.
 */
void CheckerTable_Undo(CheckerTable* Table)
{
    if (Table->Snapshot)
    {
        CheckerPieceHistory* Tmp = CPH_Transfer(&Table->Snapshot, &Table->Old); // Transfère l'élément le plus récent de l'historique des snapshots vers l'historique des anciens états

        Table->Final = Table->Old->Table;  // Met à jour l'état final avec l'état de l'ancien élément
        Table->Active = Table->Old->Table; // Met à jour l'état actif avec l'état de l'ancien élément

        // CheckerTable_Print(&Table->Final.Pieces); // Affiche l'état final (debug)
        CheckerTable_NextTurn(Table); // Passe au prochain tour
    }
}

/*
 * Entrée:
 * Sortie:
 * Fonction:  Cette fonction rétablit le dernier
 mouvement annulé en restaurant l'état suivant du tableau de jeu.
 */
void CheckerTable_Redo(CheckerTable* Table)
{
    if (Table->Old)
    {
        CheckerPieceHistory* Tmp = CPH_Transfer(&Table->Old, &Table->Snapshot); // Transfère l'élément le plus récent de l'historique des anciens états vers l'historique des snapshots

        Table->Final = Tmp->Table;  // Met à jour l'état final avec l'état de l'élément transféré
        Table->Active = Tmp->Table; // Met à jour l'état actif avec l'état de l'élément transféré

        // CheckerTable_Print(&Table->Final.Pieces); // Affiche l'état final (debug)
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
