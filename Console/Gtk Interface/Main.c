#include <gtk/gtk.h>
#include <stdio.h>
#include "Creator/Widgets.h"
#include "Creator/Widgets/Toolbar.h"


void OnInputInsert(WidgetLayout* Layout)
{
	printf("ff\n");
}

void FillRandomInOutput(WidgetLayout* Layout)
{
	GtkWidget* OuputText = WidgetLayout_GetElementById(Layout, "OuputText")->Widget;
	GtkTextBuffer* Buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(OuputText));

	gchar test[] = "\nhello world lorem ipsum";
	for (int i = 0; i < 100; i++)
	{
		GtkTextIter Iter;
		gtk_text_buffer_get_end_iter(Buf, &Iter);
		gtk_text_buffer_insert(Buf, &Iter, test, strlen(test));
	}
}


int main(int argc, char** argv)
{
	gtk_init(&argc, &argv);

	XmlDocument* Doc = XML_OpenFile("test.xml");
	WidgetLayout* Layout = WidgetLayout_ReadXml(Doc);

	FillRandomInOutput(Layout);

	//gchar test[] = "\nhello world lorem ipsum";
	//for (int i = 0; i < 100; i++)
	//{
	//	GtkTextIter Iter;
	//	gtk_text_buffer_get_end_iter(Buf, &Iter);
	//	gtk_text_buffer_insert(Buf, &Iter, test, strlen(test));
	//}

	////

	//GtkAccelGroup* Accel = gtk_accel_group_new();
	//gtk_window_add_accel_group(GTK_WINDOW(Layout->WindowList->Widget), Accel);

	//{
	//	GClosure* InputInsert = g_cclosure_new(G_CALLBACK(OnInputInsert), Layout, NULL);
	//	gtk_accel_group_connect(Accel,
	//		GDK_KEY_a,
	//		0,
	//		0,
	//		InputInsert);
	//}

	gtk_main();
	WidgetLayout_Free(Layout);

	return 0;
}
