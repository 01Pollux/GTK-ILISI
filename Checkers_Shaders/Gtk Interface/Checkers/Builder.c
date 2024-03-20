#include "Bot.h"

gboolean CheckerTable_Timer(CheckerTable* Table)
{
    if (Table->Paused)
        return TRUE;

    int Idx = Table->Turn == CPT_White ? 0 : 1;
    Table->NumberOfSeconds[Idx]--;

    static char Buffer[128];
    sprintf(Buffer, "(Red: %.2d:%.2d -- Black: %.2d:%.2d)", Table->NumberOfSeconds[0] / 60, Table->NumberOfSeconds[0] % 60, Table->NumberOfSeconds[1] / 60, Table->NumberOfSeconds[1] % 60);
    gtk_label_set_text(GTK_LABEL(Table->TimerLabel), Buffer);

    if (Table->NumberOfSeconds[Idx] == 0)
    {
        Table->Paused = TRUE;
        Table->NeedNetUpdate = TRUE;
        CheckerTable_DestroyFuture(Table);
        CheckerTable_EndGame(Table, Table->Turn == CPT_White ? CPT_Black : CPT_White);
        return FALSE;
    }

    return TRUE;
}

//

static uint8_t* CheckerTable_CreateBoard(
    int X,
    int Y,
    uint8_t FirstColor[3],
    uint8_t SecondColor[3],
    uint32_t Width,
    uint32_t Height);

static void CheckerTable_CreateBoardImage(CheckerTable* Table);

GtkWidget* CheckerTable_CreatePiece(CheckerPiece* Piece);

static GtkWidget* CheckerTable_CreatePieceFuture();

//

gboolean CheckerTable_OnTablePress(GtkWidget* Window, GdkEventButton Event, CheckerTable* Table);

gboolean CheckerTable_OnMouseMove(GtkWidget* Widget, GdkEventMotion* Event, CheckerTable* Table);

gboolean CheckerTable_OnTableCommit(GtkWidget* self, GdkEventKey Event, CheckerTable* Table);

//

static void CheckerTable_OnUndo(GtkWidget* Widget, CheckerTable* Table)
{
    CheckerTable_Undo(Table);
}

static void CheckerTable_OnRedo(GtkWidget* Widget, CheckerTable* Table)
{
    CheckerTable_Redo(Table);
}

static void CheckerTable_OnPaused(GtkWidget* Widget, CheckerTable* Table)
{
    Table->Paused = !Table->Paused;
    Table->NeedNetUpdate = TRUE;
    CheckerTable_DestroyFuture(Table);
}

/*
Nom Fonction : CheckerTable_Init
Entree : La structure de notre Table, Le Mode et La Difficulte.
Sortie :
Description : creer et initialiser la table du jeu.
*/
void CheckerTable_Init(CheckerTable* Table, CheckerPieceMode Mode, CheckerPieceDifficulty Difficulty)
{
    // 0 = vide
    // 1 = blanc
    // 2 = noir
    // 3 = blanc dame
    // 4 = noir dame
    int Board[CHECKER_TABLE_Y][CHECKER_TABLE_X] = {
        { 0, 1, 0, 1, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 1, 0, 1, 0, 1, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 2, 0, 2, 0, 2, 0, 2, 0 },
        { 0, 2, 0, 2, 0, 2, 0, 2 },
        { 2, 0, 2, 0, 2, 0, 2, 0 }
    };

    // Initialiser la table a null
    memset(Table, 0, sizeof(CheckerTable));

    Table->NumberOfSeconds[0] = 5 * 60;
    Table->NumberOfSeconds[1] = 5 * 60;

    // Initialiser mode et difficulte
    Table->Mode = Mode;
    Table->Difficulty = Difficulty;

    // Creation de la fenetre
    WindowDesc WinDesc;
    WindowDesc_Init(&WinDesc);
    WinDesc.Width = CHECKER_TABLE_X * CHECKER_PIECE_SIZE;
    WinDesc.Height = CHECKER_TABLE_Y * CHECKER_PIECE_SIZE - 10;
    Table->Window = GTK_WINDOW(Window_Create(&WinDesc));

    // Creation de Headbar
    HeaderbarDesc HeaderDesc;
    HeaderbarDesc_Init(&HeaderDesc);
    HeaderDesc.Layout = "icon:close";
    HeaderDesc.Title = "";
    GtkWidget* Header = Headerbar_Create(&HeaderDesc);
    gtk_window_set_titlebar(Table->Window, Header);

    LabelDesc label;
    LabelDesc_Init(&label);
    label.Text = "Score ( Red: 0 -- Black: 0 )";
    Table->ScoreLabel = Label_Create(&label);

    label.Text = "(Red: 5:00 -- Black: 5:00)";
    Table->TimerLabel = Label_Create(&label);

    gtk_header_bar_pack_start(GTK_HEADER_BAR(Header), Table->TimerLabel);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(Header), Table->ScoreLabel);

    // bouttons Undo ,Redo dans le headerbar.
    GtkWidget* Redo = Button_Label_Create(">");
    GtkWidget* Undo = Button_Label_Create("<");
    GtkWidget* Pause = Button_Label_Create("Pause");

    gtk_header_bar_pack_end(GTK_HEADER_BAR(Header), Redo);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(Header), Undo);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(Header), Pause);

    // Initialisation de la zone du jeu
    BoxDesc BoxDesc;
    BoxDesc_Init(&BoxDesc);
    BoxDesc.Fixed = TRUE;

    Table->Fixed = GTK_FIXED(Box_Create(&BoxDesc));
    CheckerTable_CreateBoardImage(Table);
    gtk_container_add(GTK_CONTAINER(Table->Window), GTK_WIDGET(Table->Fixed));

    // Initialisation des pieces avant de creer les images
    for (int i = 0; i < CHECKER_PIECE_COUNT; i++)
    {
        Table->Active.BlackPieces[i].Pos.X = -1;
        Table->Active.BlackPieces[i].Pos.Y = -1;
        Table->Active.WhitePieces[i].Pos.X = -1;
        Table->Active.WhitePieces[i].Pos.Y = -1;
    }
    // Creer les images des pieces et inisialiser leurs positions a partire de la matrice.
    int IdBlack = 0;
    int IdWhite = 0;

    // Parcourire la matrice du jeu
    for (char i = 0; i < CHECKER_TABLE_Y; i++)
    {
        for (char j = 0; j < CHECKER_TABLE_X; j++)
        {
            GtkWidget* Widget = NULL;
            CheckerPiece* Piece = &Table->Active.Pieces.Mat[i][j];

            switch (Board[i][j])
            {
            case 0: // Case vide
                Table->Active.Pieces.Mat[i][j].Index = -1;
                continue;

            case 1: // Case occupe par une piece Noir
            case 3:
                Piece->IsQueen = Board[i][j] == 3; // si une dame
                Piece->Team = CPT_Black;
                Piece->Index = IdBlack;
                Piece->WasQueen = Piece->IsQueen;
                // creer l'image de la piece
                Widget = CheckerTable_CreatePiece(Piece);
                Table->Active.BlackPieces[IdBlack].Image = Widget;
                Table->Active.BlackPieces[IdBlack].Pos.Y = i;
                Table->Active.BlackPieces[IdBlack].Pos.X = j;

                IdBlack++;
                break;

            case 2: // Case occupe par une piece Blanche
            case 4:
                Piece->Team = CPT_White;
                Piece->Index = IdWhite;
                Piece->IsQueen = Board[i][j] == 4; // si une dame
                Piece->WasQueen = Piece->IsQueen;
                // creer l'image de la piece
                Widget = CheckerTable_CreatePiece(Piece);
                Table->Active.WhitePieces[IdWhite].Image = Widget;
                Table->Active.WhitePieces[IdWhite].Pos.Y = i;
                Table->Active.WhitePieces[IdWhite].Pos.X = j;

                IdWhite++;
                break;
            }

            uint32_t X = j * CHECKER_PIECE_SIZE;
            uint32_t Y = i * CHECKER_PIECE_SIZE;
            gtk_fixed_put(GTK_FIXED(Table->Fixed), Widget, X, Y);
        }
    }

    Table->Final = Table->Active;
    // creer l'image du deplacement de la piece
    Table->FuturePiece = CheckerTable_CreatePieceFuture();
    gtk_fixed_put(GTK_FIXED(Table->Fixed), Table->FuturePiece, 0, 0);
    gtk_widget_hide(Table->FuturePiece);

    // Commencer par le blanc
    CheckerTable_SetTurn(Table, CPT_White);

    // Evenements de click de la souris
    g_signal_connect(Table->Window, "button-release-event", G_CALLBACK(CheckerTable_OnTablePress), Table);
    g_signal_connect(Table->Window, "key-press-event", G_CALLBACK(CheckerTable_OnTableCommit), Table);
    g_signal_connect(Table->Window, "motion_notify_event", G_CALLBACK(CheckerTable_OnMouseMove), Table);

    switch (Mode)
    {
    case CPM_BotVsBot:
    case CPM_PlayerVsBot:
        g_timeout_add(750 * (Table->Mode), G_SOURCE_FUNC(CheckerTable_BotVsBot), Table);
    }
    // Evenements des boutton de undo et redo dans le headbar
    g_signal_connect(Undo, "clicked", G_CALLBACK(CheckerTable_OnUndo), Table);
    g_signal_connect(Redo, "clicked", G_CALLBACK(CheckerTable_OnRedo), Table);
    g_signal_connect(Pause, "clicked", G_CALLBACK(CheckerTable_OnPaused), Table);

    CheckerTable_Snapshot(Table);

    g_timeout_add(1000, G_SOURCE_FUNC(CheckerTable_Timer), Table);
}

/*
Nom Fonction : CheckerTable_CreateBoard
Entree : les dimensions de la table du jeu et le theme (les couleurs)
Sortie : Image de la table
Description : Fonction qui creer dynamiquement l'image pour table de gtk
*/
uint8_t* CheckerTable_CreateBoard(
    int X,
    int Y,
    uint8_t FirstColor[3],
    uint8_t SecondColor[3],
    uint32_t Width,
    uint32_t Height)
{
    const uint32_t PixelSize = 4;

    const size_t RowPitch = Width * PixelSize;
    const size_t CellPitch = RowPitch / X;
    const size_t CellHeight = Width / X;
    const size_t TextureSize = RowPitch * Height;

    uint8_t* Chessboard = (uint8_t*)malloc(TextureSize * sizeof(uint8_t));
    Malloc_Valid(Chessboard, "CheckerTable_CreateBoard");

    size_t x, y, i, j;

    for (size_t Pixel = 0; Pixel < TextureSize; Pixel += PixelSize)
    {
        x = Pixel % RowPitch;
        y = Pixel / RowPitch;
        i = x / CellPitch;
        j = y / CellHeight;

        if ((i % 2) == (j % 2))
        {
            Chessboard[Pixel + 0] = FirstColor[0]; // R
            Chessboard[Pixel + 1] = FirstColor[1]; // G
            Chessboard[Pixel + 2] = FirstColor[2]; // B
        }
        else
        {
            Chessboard[Pixel + 0] = SecondColor[0]; // R
            Chessboard[Pixel + 1] = SecondColor[1]; // G
            Chessboard[Pixel + 2] = SecondColor[2]; // B
        }

        Chessboard[Pixel + 3] = 0xFF; // A
    }

    return Chessboard;
}

/*
Nom Fonction : CheckerTable_CreateBoardImage
Entree : la structure de la table du jeu
Sortie :
Description : Fonction qui creer l'image de la table et l'afficher
*/
void CheckerTable_CreateBoardImage(CheckerTable* Table)
{
    uint32_t CHECKER_TABLE_WIDTH = CHECKER_PIECE_SIZE * CHECKER_TABLE_X;
    uint32_t CHECKER_TABLE_HEIGHT = CHECKER_PIECE_SIZE * CHECKER_TABLE_Y;
    // definire le theme de la table (Noir et Blanc...)
    uint8_t FirstColor[3] = { CHECKER_TABLE_COLOR_WHITE };
    uint8_t SecondColor[3] = { CHECKER_TABLE_COLOR_BLACK };
    // creer l'image
    uint8_t* ColorTable = CheckerTable_CreateBoard(
        CHECKER_TABLE_X,
        CHECKER_TABLE_Y,
        FirstColor,
        SecondColor,
        CHECKER_TABLE_WIDTH,
        CHECKER_TABLE_HEIGHT);

    GdkPixbuf* Buf = gdk_pixbuf_new_from_data(
        ColorTable,
        GDK_COLORSPACE_RGB,
        1,
        8,
        CHECKER_TABLE_WIDTH,
        CHECKER_TABLE_HEIGHT,
        CHECKER_TABLE_WIDTH * 4,
        NULL, NULL);
    // afficher l'image
    GtkWidget* Image = gtk_image_new_from_pixbuf(Buf);
    gtk_widget_show(Image);
    gtk_fixed_put(Table->Fixed, Image, 0, 0);
}

/*
Nom Fonction : CheckerTable_CreatePiece
Entree : un pointeur sur la piece
Sortie : Image d'une piece
Description : Creer une image pour une piece Blanch/Noir Dame/Pion
*/
GtkWidget* CheckerTable_CreatePiece(CheckerPiece* Piece)
{
    // initialiser l'image
    ImageDesc Desc;
    ImageDesc_Init(&Desc);
    // dimensions de l'image
    Desc.Width = CHECKER_PIECE_SIZE;
    Desc.Height = CHECKER_PIECE_SIZE;

    if (Piece->Team == CPT_White)
    {
        Desc.Path = Piece->IsQueen ? CHECKER_PIECE_IMG_WHITE_QUEEN : CHECKER_PIECE_IMG_WHITE;
    }
    else
    {
        Desc.Path = Piece->IsQueen ? CHECKER_PIECE_IMG_BLACK_QUEEN : CHECKER_PIECE_IMG_BLACK;
    }
    return Image_Create(&Desc);
}

/*
Nom Fonction : CheckerTable_CreatePieceFuture
Entree :
Sortie : Image de deplacement d'une piece (position future)
Description :
*/
GtkWidget* CheckerTable_CreatePieceFuture()
{
    // initialisation
    ImageDesc Desc;
    ImageDesc_Init(&Desc);
    // dimensions
    Desc.Width = CHECKER_PIECE_SIZE;
    Desc.Height = CHECKER_PIECE_SIZE;
    // lien de l'image
    Desc.Path = CHECKER_PIECE_IMG_FUTURE;
    return Image_Create(&Desc);
}

/*
Nom Fonction : CheckerTable_QueenUpdate
Entree : la structure de la table du jeu et un pointeur de type
         CheckerPeaceRel (contient la position et un pointeur sur l'image)
Sortie :
Description :changer l'image d'une piece s'elle est devenue une dame
*/
void CheckerTable_QueenUpdate(CheckerTable* Table, CheckerPieceRel* Rel)
{
    // pointeur sur la piece dans la matrice CheckerPiece
    CheckerPiece* Piece = &Table->Active.Pieces.Mat[Rel->Pos.Y][Rel->Pos.X];
    CheckerPiece* PieceFin = &Table->Final.Pieces.Mat[Rel->Pos.Y][Rel->Pos.X];
    // pointeur sur la piece dans la table des pieces WhitePieces/BlackPieces
    CheckerPieceRel* RelFin = Piece->Team == CPT_White ? &Table->Final.WhitePieces[Piece->Index] : &Table->Final.BlackPieces[Piece->Index];

    // si la piece est devenue une dame
    if (Piece->IsQueen != Piece->WasQueen)
    {
        if (CheckerPieceRel_Exists(Rel) && Rel->Image)
        {
            // supprimer l'encienne image
            gtk_container_remove(GTK_CONTAINER(Table->Fixed), Rel->Image);
        }

        int AbsX, AbsY;
        // Recuperer la position de la piece
        CheckerTable_AbsolutePosition(Rel->Pos, &AbsX, &AbsY);
        // creer la nouvelle image de la piece (dame)
        Piece->WasQueen = Piece->WasQueen;
        Rel->Image = CheckerTable_CreatePiece(Piece);
        gtk_fixed_put(Table->Fixed, Rel->Image, AbsX, AbsY);

        // Copier les changements de la table active vers la table finale
        *RelFin = *Rel;     // modifer dans la table des pieces WhitePieces/BlackPieces
        *PieceFin = *Piece; // modifer la matrice finale
    }
}