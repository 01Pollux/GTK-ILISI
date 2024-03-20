#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmsystem.h>
#include "Checker.h"
#include <stdint.h>
#include "Bot.h"

// #define DEPTH

/**
 * Fonction:	CheckerTable_Count
 *	cette fonction permet de Calculer le score de joueur .
 *	Entree :
 *  Table : la Table de jeu de dame, de type "CheckerPieceInfo*".
 *  Team  : Variable enum qui indique le type des pion (du joueur ou de l'adversaire).
 *	Sortie :
 *  Count : Score du joueur .
 */
static int CheckerTable_Count(CheckerPieceInfo* Table, CheckerPieceTeam Team)
{
    // Le score est initialisee par une valeur aleatoire
    // Pour donner a chaque fois des resultats differents
    int Score = rand() % 100 - 5;
    CheckerPieceRel* Rel;

    // On suppose que notre equipe est blanc
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        Rel = &Table->WhitePieces[i];
        // si le pion existe
        if (CheckerPieceRel_Exists(Rel))
        {
            Score += 1;
            // si la piece est une dame
            if (Table->Pieces.Mat[Rel->Pos.Y][Rel->Pos.X].IsQueen)
            {
                Score += 60;
            }
        }

        Rel = &Table->BlackPieces[i];
        if (CheckerPieceRel_Exists(Rel))
        {
            Score -= 1;
            if (Table->Pieces.Mat[Rel->Pos.Y][Rel->Pos.X].IsQueen)
            {
                Score -= 60;
            }
        }
    } // fin de for

    // Si l'equipe est noire, on inverse le score
    if (Team != CPT_White)
    {
        Score = -Score;
    }

    return Score;
}
/**
 * Fonction:	CheckerTable_MakeAutoMove
 *  Cette fonction applique l'algorithme mini-max,
 *  Calcule le score et renvoie le meilleur deplacement que le joueur de l'ordinateur peut faire.
 *	Entree :
 *  Depth : profondeur de l'arbre des scores
 *  Team  : Variable enum qui indique le type des pion (du joueur ou de l'adversaire).
 *  DoMax : Variable boolean pour indique si on va prendre la valeur maximale ou minimale.
 *	Sortie :
 *  TopScore : la table du meilleur score .
 *  -(int) : Score du joueur .
 */
static int CheckerTable_MakeAutoMove(CheckerPieceInfo* Pieces, CheckerPieceTeam Team, int Depth, CheckerPieceInfo* TopScore, gboolean DoMax)
{
    // Si on a arrive a la profondeur
    if (!Depth)
    {
        // retourner le Score
        return CheckerTable_Count(Pieces, Team);
    }

    int Bests = 0;

    // Tableau pour stocker toutes les damiers possible
    CheckerPieceInfo* BestScore = NULL;

    // L'adversaire du joueur courant
    CheckerPieceTeam InvTeam = Team == CPT_White ? CPT_Black : CPT_White;

    // Le premier pion
    CheckerPieceRel* Rel = Team == CPT_White ? &Pieces->WhitePieces[0] : &Pieces->BlackPieces[0];

    // Initialisation d'arbre des positions
    CheckerPositionTree Positions = { 0 };

    // Parcours de toutes les pions
    for (int Count = 0; Count < CHECKER_PIECE_COUNT; Count++, Rel++)
    {
        // Si le pion n'existe pas
        if (!CheckerPieceRel_Exists(Rel))
        {
            // continuer la boucle
            continue;
        }

        // Prendre les chemins ou positions possible d'apres la position de pion
        CheckerTable_GetFutures(Pieces, Rel->Pos, &Positions, FALSE);

        // Parcours toutes les chemins

        for (int i = 0; i < Positions.TopLeftCount; i++, Bests++)
        {
            // augmentation de la taille de tableau
            BestScore = (CheckerPieceInfo*)realloc(BestScore, (Bests + 1) * sizeof(CheckerPieceInfo));

            // Creer une copie de la table
            BestScore[Bests] = *Pieces;

            // faire une mouvement dans la table courante
            CheckerTable_MovePiece(&BestScore[Bests], Team, Rel->Pos, Positions.TopLeft[i]);
        } // fin de for

        for (int i = 0; i < Positions.TopRightCount; i++, Bests++)
        {
            BestScore = (CheckerPieceInfo*)realloc(BestScore, (Bests + 1) * sizeof(CheckerPieceInfo));

            // Creer une copie de la table
            BestScore[Bests] = *Pieces;

            // faire une mouvement dans la table courante
            CheckerTable_MovePiece(&BestScore[Bests], Team, Rel->Pos, Positions.TopRight[i]);
        } // fin de for

        for (int i = 0; i < Positions.BottomLeftCount; i++, Bests++)
        {
            BestScore = (CheckerPieceInfo*)realloc(BestScore, (Bests + 1) * sizeof(CheckerPieceInfo));

            // Creer une copier de la table
            BestScore[Bests] = *Pieces;

            CheckerTable_MovePiece(&BestScore[Bests], Team, Rel->Pos, Positions.BottomLeft[i]);
        } // fin de for

        for (int i = 0; i < Positions.BottomRightCount; i++, Bests++)
        {
            BestScore = (CheckerPieceInfo*)realloc(BestScore, (Bests + 1) * sizeof(CheckerPieceInfo));

            // Creer une copier de la table
            BestScore[Bests] = *Pieces;

            CheckerTable_MovePiece(&BestScore[Bests], Team, Rel->Pos, Positions.BottomRight[i]);
        } // fin de for

        // liberer les positions
        CheckerPositionTree_Free(&Positions);
    }

    int BestScoreIndex = 0;
    int BestScoreValue = DoMax ? INT32_MIN : INT32_MAX;
    int CurScore;

    // Parcourir
    for (int i = 0; i < Bests; i++)
    {
        // Appelle recursive avec la profondeur - 1 pour prendre le meilleur score
        // de toutes les cas
        CurScore = CheckerTable_MakeAutoMove(&BestScore[i], InvTeam, Depth - 1, NULL, !DoMax);
        // Si on va prendre le max (joueur Ordinateur)
        if (DoMax)
        {
            if (CurScore >= BestScoreValue)
            {
                BestScoreValue = CurScore;
                BestScoreIndex = i;
            }
        }
        // Si on va prendre le min (joueur )
        else
        {
            if (CurScore <= BestScoreValue)
            {
                BestScoreValue = CurScore;
                BestScoreIndex = i;
            }
        }
    } // fin de for

    // s'il ya des Scores
    if (Bests)
    {
        // prendre le meilleur Score
        if (TopScore)
            *TopScore = BestScore[BestScoreIndex];
        // liberer de l'espace
        free(BestScore);
    }
    // retourner la valeur du score
    return BestScoreValue;
}

/**
 * Fonction:	CheckerTable_BotTurn
 *  Cette fonction permet de faire un tour de joueur ordinateur
 *	Entree :
 *  Table : la Table de jeu de dame, de type "CheckerPieceInfo*".
 *	Sortie :
 *  -(gboolean) : variable boolean qui retourne TRUE si tous est bien, sinon FALSE.
 */

gboolean CheckerTable_BotTurn(CheckerTable* Table)
{
    CheckerPieceInfo TopScore = { 0 };

    // Calculer le Score et retourner le meilleur emplacement dans la table
    // si le Score est de la valeur minimale alors l'ordinateur est perdu
    if (CheckerTable_MakeAutoMove(&Table->Final, Table->Turn, Table->Difficulty, &TopScore, TRUE) == INT_MIN)
    {
        // preparer la fin du jeu
        CheckerTable_EndGame(Table, Table->Team == CPT_White ? CPT_Black : CPT_White);
        return FALSE;
    }

    // affecter la nouvelle table a la table actuelle
    Table->Final = TopScore;
    Table->Active = TopScore;

    // afficher nouvelle table
    CheckerTable_Refresh(Table);

    // preparer la nouvelle tour
    CheckerTable_SetTurn(Table, Table->Turn == CPT_Black ? CPT_White : CPT_Black);
    return TRUE;
}
/**
 * Fonction:	CheckerTable_BotVsBot
 *  fonction qui implemente un joueur ordinateur contre joueur ordinateur
 *	Entree :
 *  Table : la Table de jeu de dame, de type "CheckerPieceInfo*".
 *	Sortie :
 *  -(gboolean) : variable boolean qui retourne TRUE si tous est bien, sinon FALSE.
 */
gboolean CheckerTable_BotVsBot(CheckerTable* Table)
{
    if (Table->Paused)
    {
        return TRUE;
    }

    if (Table->Mode == CPM_PlayerVsBot)
    {
        if (Table->Team == Table->Turn)
        {
            return TRUE;
        }
    }

    if (CheckerTable_TryEndGame(Table))
    {
        return FALSE;
    }

    // faire un deplacement
    gboolean Ret = CheckerTable_BotTurn(Table);
    if (Ret)
    {
        CheckerTable_PlaySound(TRUE);
        CheckerTable_Snapshot(Table);
    }
    return Ret;
}
