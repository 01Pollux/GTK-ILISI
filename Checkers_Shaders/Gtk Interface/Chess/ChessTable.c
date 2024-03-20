#include "ChessTable.h"

static uint8_t* Chess_CreateBoard(
    uint8_t FirstColor[3],
    uint8_t SecondColor[3],
    uint32_t Width,
    uint32_t Height)
{
    const uint32_t PixelSize = 4;

    const uint32_t WidthDivisions = 8;
    const uint32_t HeightDivisions = 8;

    const size_t RowPitch = Width * PixelSize;
    const size_t CellPitch = RowPitch / WidthDivisions;
    const size_t CellHeight = Width / HeightDivisions;
    const size_t TextureSize = RowPitch * Height;

    uint8_t* Chessboard = (uint8_t*)malloc(TextureSize * sizeof(uint8_t));
    if (!Chessboard)
    {
        printf("Erreur d'allocation\n");
        exit(-1);
    }

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

//

static gboolean Chess_OnTablePress(GtkWidget* Window, GdkEventButton Event, ChessTable* Table)
{
    if (Event.type != GDK_BUTTON_PRESS || Table->ClickTime == Event.time)
        return FALSE;

    if (Table->EndGame)
    {
        return FALSE;
    }

    char X = (char)((Event.x - 30) / Table->CellWidth);
    char Y = (char)((Event.y - 60) / Table->CellHeight);

    if (X < 0 || Y < 0)
        return FALSE;

    Table->ClickTime = Event.time;
    if (!Table->ActivePiece)
    {
        Table->ActivePiece = Chess_GetPiece(Table, CGT_SameTeam, X, Y, NULL);
    }
    else
    {
        Chess_ProcessInput(Table, X, Y);
    }

    return FALSE;
}

//

static ChessPieceList* Chess_CreatePiece(ChessTable* Table, ChessPieceType Type, int IsBlack, char X, char Y)
{
    const char* FileName = NULL;
    switch (Type)
    {
    case CPT_Pawn:
        FileName = IsBlack ? "Chess/Images/b_pawn_png_shadow_128px.png" : "Chess/Images/w_pawn_png_shadow_128px.png";
        break;
    case CPT_Knight:
        FileName = IsBlack ? "Chess/Images/b_knight_png_shadow_128px.png" : "Chess/Images/w_knight_png_shadow_128px.png";
        break;
    case CPT_Bishop:
        FileName = IsBlack ? "Chess/Images/b_bishop_png_shadow_128px.png" : "Chess/Images/w_bishop_png_shadow_128px.png";
        break;
    case CPT_Rook:
        FileName = IsBlack ? "Chess/Images/b_rook_png_shadow_128px.png" : "Chess/Images/w_rook_png_shadow_128px.png";
        break;
    case CPT_Queen:
        FileName = IsBlack ? "Chess/Images/b_queen_png_shadow_128px.png" : "Chess/Images/w_queen_png_shadow_128px.png";
        break;
    case CPT_King:
        FileName = IsBlack ? "Chess/Images/b_king_png_shadow_128px.png" : "Chess/Images/w_king_png_shadow_128px.png";
        break;
    }

    ChessPieceList* Piece = (ChessPieceList*)calloc(1, sizeof(ChessPieceList));
    Malloc_Valid(Piece, "Chess_CreatePiece\n");

    Piece->XPos = X;
    Piece->YPos = Y;
    Piece->Type = Type;
    Piece->Team = IsBlack ? CTT_Black : CTT_White;

    GdkPixbuf* Buf = gdk_pixbuf_new_from_file_at_scale(FileName, Table->CellWidth, Table->CellHeight, 0, NULL);
    Piece->Image = gtk_image_new_from_pixbuf(Buf);
    gtk_widget_show(Piece->Image);

    Table->Pieces[Y][X] = Piece;

    return Piece;
}

static ChessPieceList* Chess_AddPiece(ChessPieceList* List, ChessPieceList* Piece)
{
    if (List)
    {
        Piece->Next = List;
    }
    List = Piece;
    return List;
}

//

static ChessPieceList* Chess_AddPieces(ChessTable* Table, int IsBlack)
{
    ChessPieceList* Pieces = NULL;

    ChessPieceType Types[] = {
        CPT_Pawn, CPT_Pawn, CPT_Pawn, CPT_Pawn, CPT_Pawn, CPT_Pawn, CPT_Pawn, CPT_Pawn,
        CPT_Rook, CPT_Knight, CPT_Bishop, CPT_Queen, CPT_King, CPT_Bishop, CPT_Knight, CPT_Rook
    };

    char PawnOffset, OtherOffset;

    if (IsBlack)
    {
        OtherOffset = 0;
        PawnOffset = 1;
    }
    else
    {
        OtherOffset = 7;
        PawnOffset = 6;
    }

    ChessPieceList* Piece;
    for (int i = 0; i < 16; i++)
    {
        char X = i % 8;
        char Y = (i / 8) > 0 ? OtherOffset : PawnOffset;

        Piece = Chess_CreatePiece(Table, Types[i], IsBlack, X, Y);
        Pieces = Chess_AddPiece(Pieces, Piece);

        gtk_fixed_put(Table->MainBox, Piece->Image, 0, 0);
    }

    return Pieces;
}

//

void Chess_Debug_AddPiece(
    ChessTable* Table,
    char IsBlack,
    ChessPieceType Type,
    char X,
    char Y)
{
    char PawnOffset, OtherOffset;
    if (IsBlack)
    {
        OtherOffset = 0;
        PawnOffset = 1;
    }
    else
    {
        OtherOffset = 7;
        PawnOffset = 6;
    }

    ChessPieceList* Piece;
    Piece = Chess_CreatePiece(Table, Type, IsBlack, X, Y);

    if (IsBlack)
        Table->BlackPieces = Chess_AddPiece(Table->BlackPieces, Piece);
    else
        Table->WhitePieces = Chess_AddPiece(Table->WhitePieces, Piece);

    int AbsX = Piece->XPos * Table->CellWidth;
    int AbsY = Piece->YPos * Table->CellHeight;

    gtk_fixed_put(Table->MainBox, Piece->Image, AbsX, AbsY);
}

void Chess_Debug_MoveKing(
    ChessTable* Table,
    char IsBlack,
    char X,
    char Y)
{
    ChessPieceList* KingPiece = IsBlack ? Table->BlackPieces : Table->WhitePieces;
    while (KingPiece && (KingPiece->Type != CPT_King))
    {
        KingPiece = KingPiece->Next;
    }

    KingPiece->XPos = X;
    KingPiece->YPos = Y;

    int AbsX = KingPiece->XPos * Table->CellWidth;
    int AbsY = KingPiece->YPos * Table->CellHeight;

    gtk_fixed_move(Table->MainBox, KingPiece->Image, AbsX, AbsY);
}

//

void Chess_CreateTable(
    ChessTable* Table,
    WidgetContainer* MainBox,
    uint8_t FirstColor[3],
    uint8_t SecondColor[3],
    uint32_t TableWidth,
    uint32_t TableHeight)
{
    //
    memset(Table, 0, sizeof(*Table));
    Table->PlayerTurn = CTT_White;

    // Get window containing box
    WidgetContainer* Window = MainBox;
    while (Window->Parent)
    {
        Window = Window->Parent;
    }

    // Hook-up press button event
    g_signal_connect(Window->Widget, "button-press-event", G_CALLBACK(Chess_OnTablePress), Table);

    //

    Table->ColorTable = Chess_CreateBoard(FirstColor, SecondColor, TableWidth, TableHeight);

    //

    GdkPixbuf* Buf = gdk_pixbuf_new_from_data(
        Table->ColorTable,
        GDK_COLORSPACE_RGB,
        1,
        8,
        TableWidth,
        TableHeight,
        TableWidth * 4,
        NULL, NULL);

    Table->ChessImage = gtk_image_new_from_pixbuf(Buf);
    gtk_widget_show(Table->ChessImage);

    //

    gtk_container_add(GTK_CONTAINER(MainBox->Widget), Table->ChessImage);
    MainBox->Children = WidgetContainer_CreateContainer(MainBox->Children, Table->ChessImage);

    //

    Table->CellWidth = TableWidth / 8;
    Table->CellHeight = TableHeight / 8;
    Table->MainBox = GTK_FIXED(MainBox->Widget);
    Table->WhitePieces = Chess_AddPieces(Table, 0);
    Table->BlackPieces = Chess_AddPieces(Table, 1);
}

void Chess_UpdateTable(
    ChessTable* Table)
{
    ChessPieceList* List = Table->WhitePieces;
    while (List)
    {
        int X = List->XPos * Table->CellWidth;
        int Y = List->YPos * Table->CellHeight;
        gtk_fixed_move(Table->MainBox, List->Image, X, Y);
        List = List->Next;
    }

    List = Table->BlackPieces;
    while (List)
    {
        int X = List->XPos * Table->CellWidth;
        int Y = List->YPos * Table->CellHeight;
        gtk_fixed_move(Table->MainBox, List->Image, X, Y);
        List = List->Next;
    }
}

ChessPieceList* Chess_GetPiece(
    ChessTable* Table,
    ChessGetPieceType Type,
    char X,
    char Y,
    int* SameTeam)
{
    ChessPieceList* Piece = NULL;
    switch (Type)
    {
    case CGT_Any:
    {
        Piece = Chess_GetPiece(Table, CGT_SameTeam, X, Y, NULL);
        if (SameTeam)
            *SameTeam = 1;
        if (Piece)
            return Piece;

        Piece = Chess_GetPiece(Table, CGT_OppositeTeam, X, Y, NULL);
        if (SameTeam)
            *SameTeam = 0;
        return Piece;
    }

    case CGT_SameTeam:
        Piece = Table->PlayerTurn == CTT_White ? Table->WhitePieces : Table->BlackPieces;
        break;

    case CGT_OppositeTeam:
        Piece = Table->PlayerTurn == CTT_White ? Table->BlackPieces : Table->WhitePieces;
        break;
    }

    while (Piece && (Piece->XPos != X || Piece->YPos != Y))
    {
        Piece = Piece->Next;
    }
    return Piece;
}

//
