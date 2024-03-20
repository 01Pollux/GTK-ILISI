#include <gtk/gtk.h>
#include "Creator/Widgets.h"

void InitializeGTK(int argc, char** argv)
{
    gtk_init(&argc, &argv);
}

void MainFonction()
{
    gtk_main();
}

//

void PopupWindowCreate(GtkWidget* Button, GtkBox* Box);

//

void QuitMain(GtkWidget* Button, gpointer Data)
{
    gtk_main_quit();
}

GtkFixed* OpenExWindow()
{
    WindowDesc WDesc;
    WindowDesc_Init(&WDesc);
    WDesc.Title = "Test des fonction GTK+";
    WDesc.Width = 1000;
    WDesc.Height = 600;
    WDesc.Resizable = FALSE;
    WDesc.Icon = "ayooo2";

    GtkWidget* Window = Window_Create(&WDesc);

    //

    HeaderbarDesc HDesc;
    HeaderbarDesc_Init(&HDesc);
    HDesc.Title = WDesc.Title;
    HDesc.Layout = "icon,menu:minimize,close,maximize";
    GtkWidget* Header = Headerbar_Create(&HDesc);

    gtk_window_set_titlebar(GTK_WINDOW(Window), Header);

    //

    ScrolledWindowDesc SDesc;
    ScrolledWindowDesc_Init(&SDesc);
    SDesc.HAdjustment.Visibility = GTK_POLICY_ALWAYS;
    SDesc.VAdjustment.Visibility = GTK_POLICY_ALWAYS;
    GtkWidget* Scrolled = ScrolledWindow_Create(&SDesc);

    gtk_container_add(GTK_CONTAINER(Window), Scrolled);

    //

    BoxDesc BDesc;
    BoxDesc_Init(&BDesc);
    BDesc.Fixed = TRUE;
    GtkWidget* Box = Box_Create(&BDesc);

    gtk_container_add(GTK_CONTAINER(Scrolled), Box);

    return GTK_FIXED(Box);
}

void L_AddToolbarToBox(GtkBox* LBox)
{
    MenuItemList* Items = NULL;

    MenuItemEntry Tmp = { 0 };
    Tmp.Icon = "2209427_82130403_p0.png";

    const char* Noms[] = {
        "Fichier",
        "Acceuil",
        "Insertion"
    };
    char str[5][20];

    for (int i = 0; i < 3; i++)
    {
        MenuItemList* TmpChildren = NULL;

        Tmp.Name = "Enregister";
        TmpChildren = MenuItemList_AddItem(TmpChildren, &Tmp, NULL);

        Tmp.Name = "Enregistrer Sous";
        TmpChildren = MenuItemList_AddItem(TmpChildren, &Tmp, NULL);

        Tmp.Name = "Enregistrer au format Adobe PDF";
        TmpChildren = MenuItemList_AddItem(TmpChildren, &Tmp, NULL);

        MenuItemList* TF = NULL;
        for (int j = 0; j < 5; j++)
        {
            sprintf(str[j], "File%i.txt", j);
            Tmp.Name = str[j];
            TF = MenuItemList_AddItem(TF, &Tmp, NULL);
        }

        Tmp.Name = "Ouvrir";
        TmpChildren = MenuItemList_AddItem(TmpChildren, &Tmp, TF);

        Tmp.Name = "Fermer";
        TmpChildren = MenuItemList_AddItem(TmpChildren, &Tmp, NULL);

        Items = MenuItemList_AddItem(Items, &Tmp, TmpChildren);
    }

    GtkWidget* Widget = MenuToolbar_CreateBar(Items, GTK_PACK_DIRECTION_LTR);
    MenuItemList_Free(Items);

    gtk_box_pack_start(LBox, Widget, TRUE, TRUE, 0);
}

void L_AddLabelToBox(GtkBox* LBox, const char* Text)
{
    LabelDesc Desc;
    LabelDesc_Init(&Desc);

    Desc.Text = Text;
    GtkWidget* Label = Label_Create(&Desc);

    gtk_widget_set_halign(Label, GTK_ALIGN_START);

    gtk_box_pack_start(LBox, Label, TRUE, TRUE, 0);
}

void L_InsertSubBox(GtkBox* SubBox)
{
    BoxDesc Desc;
    BoxDesc_Init(&Desc);
    Desc.Orientation = GTK_ORIENTATION_HORIZONTAL;

    GtkWidget* Box = Box_Create(&Desc);
    gtk_box_pack_start(SubBox, Box, TRUE, TRUE, 0);

    gtk_widget_set_halign(Box, GTK_ALIGN_START);

    L_AddLabelToBox(GTK_BOX(Box), "Choix parmis 5");

    ComboBoxItem* Items = NULL;
    Items = ComboBoxItem_Add(Items, "choix 1");
    Items = ComboBoxItem_Add(Items, "choix 2");
    Items = ComboBoxItem_Add(Items, "choix 3");
    Items = ComboBoxItem_Add(Items, "choix 4");
    Items = ComboBoxItem_Add(Items, "choix 5");

    GtkWidget* ComboBox = ComboBox_Create(Items);
    ComboBoxItem_Free(Items);

    gtk_box_pack_start(GTK_BOX(Box), ComboBox, TRUE, TRUE, 0);

    //
    //
    //

    Box = Box_Create(&Desc);
    gtk_box_pack_end(SubBox, Box, TRUE, TRUE, 0);

    gtk_widget_set_halign(Box, GTK_ALIGN_START);

    L_AddLabelToBox(GTK_BOX(Box), "Choix parmis 7");

    Items = NULL;
    Items = ComboBoxItem_Add(Items, "choix 1");
    Items = ComboBoxItem_Add(Items, "choix 2");
    Items = ComboBoxItem_Add(Items, "choix 3");
    Items = ComboBoxItem_Add(Items, "choix 4");
    Items = ComboBoxItem_Add(Items, "choix 5");
    Items = ComboBoxItem_Add(Items, "choix 6");
    Items = ComboBoxItem_Add(Items, "choix 7");

    ComboBox = ComboBox_Create(Items);
    ComboBoxItem_Free(Items);

    gtk_box_pack_start(GTK_BOX(Box), ComboBox, TRUE, TRUE, 0);
}

void L_InsertOkAndQut(GtkBox* SubBox)
{
    BoxDesc Desc;
    BoxDesc_Init(&Desc);
    Desc.Orientation = GTK_ORIENTATION_HORIZONTAL;

    GtkWidget* Box = Box_Create(&Desc);
    gtk_box_pack_start(SubBox, Box, TRUE, TRUE, 0);

    GtkWidget* Ok = Button_Label_Create("Ok");
    GtkWidget* Qut = Button_Label_Create("Quitter");

    Widget_AddStyle(Ok, "* {background-image: url('2209427_83125267_p0.png'); }");
    Widget_AddStyle(Qut, "* {background-image: url('2209427_83125267_p0.png'); }");

    gtk_box_pack_start(GTK_BOX(Box), Ok, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(Box), Qut, TRUE, TRUE, 0);

    g_signal_connect(Ok, "clicked", G_CALLBACK(PopupWindowCreate), SubBox);
    g_signal_connect(Qut, "clicked", G_CALLBACK(QuitMain), NULL);
}

void L_AddRadios(GtkBox* LBox)
{
    BoxDesc Desc;
    BoxDesc_Init(&Desc);
    GtkWidget* Box = Box_Create(&Desc);

    gtk_widget_set_halign(Box, GTK_ALIGN_START);
    gtk_box_pack_start(LBox, Box, TRUE, TRUE, 0);
    Widget_AddStyle(Box, "*{border: black 2px solid}");

    const char* Grades[] = {
        "Faible",
        "Moyen",
        "Assez Bien",
        "Bien",
        "Tres Bien"
    };
    GtkWidget* Link = NULL;
    RadioButtonDesc RDesc;

    for (int i = 0; i < 5; i++)
    {
        RadioButtonDesc_Init(&RDesc);
        RDesc.Label = Grades[i];
        RDesc.Link = Link;
        GtkWidget* RB = Button_Radio_Create(&RDesc);
        Link = RB;
        gtk_box_pack_start(GTK_BOX(Box), RB, TRUE, TRUE, 0);
        gtk_widget_set_halign(RB, GTK_ALIGN_START);
        Widget_AddStyle(RB, "radio {border: black 2px solid}");
    }
}

void InsertLeftBox(GtkBox* LBox)
{
    L_AddToolbarToBox(LBox);
    L_AddLabelToBox(LBox, "Liste Deroulante Choix");

    BoxDesc Desc;
    BoxDesc_Init(&Desc);
    GtkWidget* Box = Box_Create(&Desc);
    gtk_widget_set_halign(Box, GTK_ALIGN_START);
    Widget_AddStyle(Box, "*{border:2px solid black}");

    gtk_box_pack_start(LBox, Box, TRUE, TRUE, 0);

    L_InsertSubBox(GTK_BOX(Box));

    L_InsertOkAndQut(LBox);

    L_AddLabelToBox(LBox, "Boutons Radio Verticales");
    L_AddRadios(LBox);
}
void R_AddLabelToBox(GtkFixed* RBox, const char* Text, gint x, gint y)
{
    LabelDesc LabelDesc;
    LabelDesc_Init(&LabelDesc);

    LabelDesc.Text = Text;
    GtkWidget* Label = Label_Create(&LabelDesc);

    gtk_fixed_put(RBox, Label, x, y);
    gtk_widget_set_halign(Label, GTK_ALIGN_END);
}

void R_CheckBoxs(GtkWidget* FcheckBox)
{
    const char* Labels[] = {
        "Mon 1er Choix",
        "Mon 2d Choix",
        "Mon 3e Choix",
        "Mon dernier Choix"
    };

    for (int i = 0; i < 4; i++)
    {
        GtkWidget* Checkbox = CheckBox_Create(Labels[i]);
        gtk_box_pack_start(GTK_BOX(FcheckBox), Checkbox, TRUE, TRUE, 5);
        Widget_AddStyle(Checkbox, "check {border: black 2px solid}");
    }
}

void R_RadioButtons(GtkWidget* FRadioBox)
{
    const char* Labels[] = {
        "Premier",
        "Deuxieme",
        "Troisieme",
    };

    GtkWidget* Link = NULL;
    for (int i = 0; i < 3; i++)
    {
        RadioButtonDesc RadioDesc;
        RadioButtonDesc_Init(&RadioDesc);
        RadioDesc.Label = Labels[i];
        RadioDesc.Link = Link;
        GtkWidget* Radio = Button_Radio_Create(&RadioDesc);
        Link = Radio;
        gtk_box_pack_start(GTK_BOX(FRadioBox), Radio, TRUE, TRUE, 5);
        Widget_AddStyle(Radio, "radio {border: black 2px solid}");
    }
}

void R_InsertLogin(GtkWidget* RBox)
{
    BoxDesc FLoginBoxDesc;
    BoxDesc_Init(&FLoginBoxDesc);
    GtkWidget* FLoginBox = Box_Create(&FLoginBoxDesc);
    gtk_fixed_put(GTK_FIXED(RBox), FLoginBox, 600, 325);
    gtk_widget_set_halign(FLoginBox, GTK_ALIGN_START);
    Widget_AddStyle(FLoginBox, "*{border: black 2px solid;}");

    TextInputDesc InputDescLog, InputDescMP;

    TextInputDesc_Init(&InputDescLog);
    TextInputDesc_Init(&InputDescMP);
    InputDescLog.HintText = "Login";
    InputDescMP.HintText = "Mot de passe";
    InputDescMP.Visible = FALSE;

    GtkWidget* InputLog = TextInput_Create(&InputDescLog);
    GtkWidget* InputMP = TextInput_Create(&InputDescMP);

    GtkWidget* Seconnecter = Button_Label_Create("Seconnecter");
    Widget_AddStyle(Seconnecter, "* {background-image: url('2209427_83125267_p0.png'); }");

    gtk_widget_set_halign(Seconnecter, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(FLoginBox), InputLog, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(FLoginBox), InputMP, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(FLoginBox), Seconnecter, TRUE, TRUE, 0);
}

void R_InsertRadioButtons(GtkWidget* RBox)
{
    BoxDesc FRadioBoxDesc;
    BoxDesc_Init(&FRadioBoxDesc);
    FRadioBoxDesc.Orientation = GTK_ORIENTATION_HORIZONTAL;
    GtkWidget* FRadioBox = Box_Create(&FRadioBoxDesc);
    Widget_AddStyle(FRadioBox, "*{border: black 2px solid;}");
    gtk_fixed_put(GTK_FIXED(RBox), FRadioBox, 600, 240);
    gtk_widget_set_halign(FRadioBox, GTK_ALIGN_START);
    R_RadioButtons(FRadioBox);
}

void R_InsertCheckBox(GtkWidget* RBox)
{
    BoxDesc FBoxDesc;
    BoxDesc_Init(&FBoxDesc);

    GtkWidget* FBox = Box_Create(&FBoxDesc);
    gtk_fixed_put(GTK_FIXED(RBox), FBox, 600, 40);

    gtk_widget_set_halign(FBox, GTK_ALIGN_START);
    Widget_AddStyle(FBox, "*{border: black 2px solid;}");

    R_CheckBoxs(FBox);
}
void InsertRightBox(GtkFixed* RBox)
{
    R_AddLabelToBox(RBox, "Liste Choix Multilpes :", 600, 0);
    R_InsertCheckBox(GTK_WIDGET(RBox));

    R_AddLabelToBox(RBox, "Boutons Radio Horizontales :", 600, 200);
    R_InsertRadioButtons(GTK_WIDGET(RBox));

    R_AddLabelToBox(RBox, "Saisie du login et de mot de passe : Zone de texte :", 600, 300);
    R_InsertLogin(GTK_WIDGET(RBox));
}

//

void AddBoxItem(GtkFixed* MainBox)
{
    BoxDesc LBoxDesc, RBoxDesc;
    BoxDesc_Init(&LBoxDesc);
    BoxDesc_Init(&RBoxDesc);
    RBoxDesc.Fixed = TRUE;

    GtkWidget* LBox = Box_Create(&LBoxDesc);
    GtkWidget* RBox = Box_Create(&RBoxDesc);

    gtk_fixed_put(MainBox, LBox, 0, 0);
    gtk_fixed_put(MainBox, RBox, 0, 0);

    InsertLeftBox(GTK_BOX(LBox));
    InsertRightBox(GTK_FIXED(RBox));
}

//

void PopupWindowCreate(GtkWidget* Button, GtkBox* Box)
{
}

//

int main(int argc, char** argv)
{
    InitializeGTK(argc, argv);
    GtkFixed* MainBox = OpenExWindow();
    AddBoxItem(MainBox);
    MainFonction();

    return 0;
}
