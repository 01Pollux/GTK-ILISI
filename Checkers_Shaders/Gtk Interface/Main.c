#include <gtk/gtk.h>
#include "Creator/Widgets.h"
#include "Checkers/Checker.h"

void InitializeGTK(int argc, char** argv)
{
    gtk_init(&argc, &argv);
}

void MainFonction()
{
    gtk_main();
}

typedef struct
{
    WidgetLayout* Layout;
    int Port;
    char Ip[16];
} OpenPopupInfo;

// hide/destroy the interface and run the game.
void OpenChecker(GtkWidget* widget, OpenPopupInfo* Popup)
{
    WidgetLayout* Layout = Popup->Layout;

    gtk_widget_hide(Layout->WindowList->Widget);

    WidgetContainer* Gamemode = WidgetLayout_GetElementById(Layout, "Gamemode");
    WidgetContainer* Difficulty = WidgetLayout_GetElementById(Layout, "Difficulty");
    WidgetContainer* StartTurn = WidgetLayout_GetElementById(Layout, "StartTurn");

    CheckerPieceMode Mode = (CheckerPieceMode)gtk_combo_box_get_active(GTK_COMBO_BOX(Gamemode->Widget));
    CheckerPieceDifficulty Diff = (CheckerPieceDifficulty)((gtk_combo_box_get_active(GTK_COMBO_BOX(Difficulty->Widget)) + 1) * 2);

    CheckerTable* Checker = (CheckerTable*)calloc(1, sizeof(CheckerTable));
    CheckerTable_Init(Checker, Mode, Diff);

    gboolean StartWhite = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(StartTurn->Widget));
    Checker->Team = StartWhite ? CPT_White : CPT_Black;

    if (Mode == CPM_PlayerVsRPC)
    {
        WidgetContainer* RpcIp = WidgetLayout_GetElementById(Layout, "ConnectToIp");
        WidgetContainer* RpcPort = WidgetLayout_GetElementById(Layout, "ConnectToPort");

        Net_GameStart(Checker, Popup->Ip, Popup->Port, gtk_entry_get_text(GTK_ENTRY(RpcIp->Widget)), atoi(gtk_entry_get_text(GTK_ENTRY(RpcPort->Widget))), StartWhite);
    }

    CheckerTable_SetTurn(Checker, CPT_White);
}

#include "Creator/Widgets/Shader.h"

int main(int argc, char** argv)
{
    OpenPopupInfo* Popup = (OpenPopupInfo*)calloc(1, sizeof(OpenPopupInfo));
    Malloc_Valid(Popup, "main");

    Net_Init(Popup->Ip, &Popup->Port);
    InitializeGTK(argc, argv);

    // loading game interface xml file
    XmlDocument* Document = XML_OpenFile("Checkers/Interface.xml");
    WidgetLayout* Layout = WidgetLayout_ReadXml(Document);
    Popup->Layout = Layout;

    // get main window and buttons
    GtkWidget* play = WidgetLayout_GetElementById(Layout, "playBTN")->Widget;
    GtkWidget* exit = WidgetLayout_GetElementById(Layout, "quitBTN")->Widget;

    // adding signals to the play and exit button
    g_signal_connect_swapped(exit, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(play, "clicked", G_CALLBACK(OpenChecker), Popup);

    MainFonction();

    return 0;
}
