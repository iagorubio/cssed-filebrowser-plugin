/*  cssed file browser plugin (c) Iago Rubio 2004
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <plugin.h>
#include "filebrowser.h"

#ifndef WIN32
# include <unistd.h>
#endif

typedef struct _FbEditListDialogData {
	CssedFileBrowser* fb;
	GtkListStore* store;
	GtkWidget* widget;
} FbDlgData;

// useful functions
GtkWidget* 
fb_create_action_widget_dialog( GtkWidget* action_widget, gchar* title )
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *dialog_action_area;
	GtkWidget *cancelbutton;
	GtkWidget *okbutton;
	
	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_widget_set_size_request (dialog, 450, 100);
	
	dialog_vbox = GTK_DIALOG (dialog)->vbox;
	gtk_widget_show (dialog_vbox);
	
	gtk_widget_show (action_widget);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), action_widget, TRUE, TRUE, 0);
	
	dialog_action_area = GTK_DIALOG (dialog)->action_area;
	gtk_widget_show (dialog_action_area);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
	
	cancelbutton = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), cancelbutton, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);
	
	okbutton = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), okbutton, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);
	
	return dialog;
}

// callbacks
void
on_fb_edit_list_dialog_close           (GtkDialog       *dialog,
                                        gpointer         user_data)
{
	gtk_widget_destroy( GTK_WIDGET(dialog) );
	g_free( user_data );
}

void
on_fb_edit_list_treeview_row_activated (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	GtkTreeIter iter;
	GtkTreeModel* model;
	GtkWidget* dialog;
	GtkWidget* entry;
	gchar* item, *edited;
	
	gint response;

	model = gtk_tree_view_get_model( treeview );
 
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		gtk_tree_model_get (model, &iter, 0, &item, -1);
		entry = gtk_entry_new();
		dialog = fb_create_action_widget_dialog( entry, _("Edit item"));	
		gtk_entry_set_text( GTK_ENTRY(entry), item );
		response = gtk_dialog_run( GTK_DIALOG(dialog) );
	
		if( response == GTK_RESPONSE_OK )
		{		
			edited = gtk_editable_get_chars( GTK_EDITABLE(entry), 0, -1 );
			if( strlen(edited) > 0 )
				gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, edited, -1);
			g_free(edited);
		}	
		gtk_widget_destroy( dialog );		
		g_free( item );
	} 
}


void
on_fb_edit_list_button_add_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* dialog;
	GtkWidget* entry;
	gchar* item;
	gint response;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeIter iter;

	entry = gtk_entry_new();
	dialog = fb_create_action_widget_dialog(entry, _("Add an item"));

	response = gtk_dialog_run( GTK_DIALOG(dialog) );

	if( response == GTK_RESPONSE_OK )
	{		
		item = gtk_editable_get_chars( GTK_EDITABLE(entry), 0, -1 );
		if( strlen(item) > 0 ){		
			treeview = GTK_TREE_VIEW(user_data);
			store = gtk_tree_view_get_model( treeview );
			gtk_list_store_append( GTK_LIST_STORE(store), &iter );
			gtk_list_store_set (GTK_LIST_STORE(store), &iter, 0, item, -1);
		}
		g_free(item);
	}	
	gtk_widget_destroy( dialog );
}


void
on_fb_edit_list_button_remove_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeIter iter, new_iter;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeSelection* selection;
	GtkTreePath* path;
	
	treeview = GTK_TREE_VIEW(user_data);
	selection = gtk_tree_view_get_selection (treeview);

	if ( gtk_tree_selection_get_selected (selection, &store, &iter) )
	{
		path = gtk_tree_model_get_path (store, &iter);
		if(  gtk_tree_path_prev (path) ){
			if( gtk_tree_model_get_iter (store, &new_iter, path) )
				gtk_tree_selection_select_iter  (selection,&new_iter);
		}
		gtk_list_store_remove (GTK_LIST_STORE(store), &iter);
	}
}

void
on_fb_edit_list_button_up_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeIter iter, new_iter;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeSelection* selection;
	GtkTreePath* path;

	treeview = GTK_TREE_VIEW(user_data);
	selection = gtk_tree_view_get_selection( treeview );

	if (gtk_tree_selection_get_selected (selection, &store, &iter))
	{
		path = gtk_tree_model_get_path ( store, &iter);
		gtk_tree_path_prev (path);
		if( gtk_tree_model_get_iter ( store, &new_iter, path) )
			gtk_list_store_swap (GTK_LIST_STORE(store), &iter, &new_iter);
	}
}


void
on_fb_edit_list_button_down_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeIter iter, new_iter;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeSelection* selection;
	GtkTreePath* path;

	treeview = GTK_TREE_VIEW(user_data);
	selection = gtk_tree_view_get_selection( treeview );

	if (gtk_tree_selection_get_selected (selection, &store, &iter))
	{
		path = gtk_tree_model_get_path ( store, &iter);
		gtk_tree_path_next (path);
		if( gtk_tree_model_get_iter ( store, &new_iter, path) )
			gtk_list_store_swap (GTK_LIST_STORE(store), &iter, &new_iter);
	}
}


void
on_fb_edit_list_button_top_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeIter iter;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeSelection* selection;

	treeview = GTK_TREE_VIEW(user_data);
	selection = gtk_tree_view_get_selection( treeview );

	if (gtk_tree_selection_get_selected (selection, &store, &iter))
		gtk_list_store_move_after (GTK_LIST_STORE(store), &iter, NULL);
}


void
on_fb_edit_list_button_bottom_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeIter iter;
	GtkTreeModel* store;
	GtkTreeView* treeview;
	GtkTreeSelection* selection;

	treeview = GTK_TREE_VIEW(user_data);
	selection = gtk_tree_view_get_selection( treeview );

	if (gtk_tree_selection_get_selected (selection, &store, &iter))
		gtk_list_store_move_before (GTK_LIST_STORE(store), &iter, NULL);
}


void
on_fb_edit_list_cancelbutton_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
	FbDlgData* data;

	data = (FbDlgData*) user_data;
	gtk_widget_destroy( data->widget );
	g_free( data );
}


void
fb_edit_list_apply_changes( FbDlgData* data )
{
	CssedFileBrowser* fb;
	GtkListStore* store;
	GtkTreeIter iter;
	gchar *item, *dir;
	GList* list = NULL;

	fb = data->fb;
	store = data->store;
	dir = gtk_editable_get_chars(GTK_EDITABLE( fb->entry_directory ), 0, -1);	
	// clean old items
	if( fb->dirlist != NULL ){
		list = g_list_first( fb->dirlist );
		while( list ){
			g_free( list->data );
			list = g_list_next( list );
		}
		g_list_free( fb->dirlist );
	}

	fb->dirlist = NULL;

	if( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter) )
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, 0, &item, -1);
		fb->dirlist = g_list_append( fb->dirlist, item );
		while( gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter) )
		{
			gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, 0, &item, -1);
			fb->dirlist = g_list_append( fb->dirlist, item );		
		}
	}
	gtk_combo_set_popdown_strings ( GTK_COMBO( fb->combo_directory ), fb->dirlist );
	
	// restore old directory because gtk_combo_set_popdown_strings
	// changes the text in the entry
	if( dir != NULL ){
		gtk_entry_set_text (GTK_ENTRY( fb->entry_directory ), dir);
		g_free( dir );
	}
}
void
on_fb_edit_list_applybutton_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	fb_edit_list_apply_changes( (FbDlgData*) user_data );
}

void
on_fb_edit_list_okbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	FbDlgData* data;
	data = (FbDlgData*) user_data;

	fb_edit_list_apply_changes( data );
	gtk_widget_destroy( data->widget );	
	g_free( user_data );
}

void
fb_edit_list_populate_store ( CssedFileBrowser* fb, GtkListStore* store )
{
	GList* list = NULL;
	GtkTreeIter iter;

	list = g_list_first( fb->dirlist );
	while( list )
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, list->data, -1);
		list = g_list_next( list );
	}
}

// build interface
GtkWidget*
fb_create_edit_list_dialog  ( CssedFileBrowser* fb )
{
	GtkWidget *edit_list_dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *vbox;
	GtkWidget *scrolledwindow;
	GtkWidget *treeview;
	GtkWidget *hbuttonbox;
	GtkWidget *button_add;
	GtkWidget *button_remove;
	GtkWidget *button_up;
	GtkWidget *button_down;
	GtkWidget *button_top;
	GtkWidget *button_bottom;
	GtkWidget *dialog_action_area;
	GtkWidget *cancelbutton;
	GtkWidget *applybutton;
	GtkWidget *okbutton;
	GtkListStore* items_store;
	GtkCellRenderer* text_renderer;
	GtkTreeViewColumn* items_col;
	FbDlgData* data;
	
	data = g_malloc0(sizeof(FbDlgData));
	
	edit_list_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (edit_list_dialog), _("Edit the drop down list"));
	
	dialog_vbox = GTK_DIALOG (edit_list_dialog)->vbox;
	gtk_widget_show (dialog_vbox);
	gtk_widget_set_size_request     (dialog_vbox, 550, 450);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), vbox, TRUE, TRUE, 0);
	
	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow);
	gtk_box_pack_start (GTK_BOX (vbox), scrolledwindow, TRUE, TRUE, 0);
	
	treeview = gtk_tree_view_new ();
	gtk_widget_show (treeview);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	
	text_renderer = gtk_cell_renderer_text_new();
	items_store = gtk_list_store_new(1, G_TYPE_STRING);
	items_col = gtk_tree_view_column_new_with_attributes ("",
													  text_renderer,
													  "text", 0,
													   NULL);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (items_store));
	g_object_unref(items_store);
	gtk_tree_view_insert_column ( GTK_TREE_VIEW(treeview), items_col, 0 );
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview),  FALSE);
	
	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbuttonbox, FALSE, FALSE, 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 2);
	
	button_add = gtk_button_new_from_stock ("gtk-add");
	gtk_widget_show (button_add);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_add);
	GTK_WIDGET_SET_FLAGS (button_add, GTK_CAN_DEFAULT);
	
	button_remove = gtk_button_new_from_stock ("gtk-remove");
	gtk_widget_show (button_remove);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_remove);
	GTK_WIDGET_SET_FLAGS (button_remove, GTK_CAN_DEFAULT);
	
	button_up = gtk_button_new_from_stock ("gtk-go-up");
	gtk_widget_show (button_up);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_up);
	GTK_WIDGET_SET_FLAGS (button_up, GTK_CAN_DEFAULT);
	
	button_down = gtk_button_new_from_stock ("gtk-go-down");
	gtk_widget_show (button_down);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_down);
	GTK_WIDGET_SET_FLAGS (button_down, GTK_CAN_DEFAULT);
	
	button_top = gtk_button_new_from_stock ("gtk-goto-top");
	gtk_widget_show (button_top);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_top);
	GTK_WIDGET_SET_FLAGS (button_top, GTK_CAN_DEFAULT);
	
	button_bottom = gtk_button_new_from_stock ("gtk-goto-bottom");
	gtk_widget_show (button_bottom);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_bottom);
	GTK_WIDGET_SET_FLAGS (button_bottom, GTK_CAN_DEFAULT);
	
	dialog_action_area = GTK_DIALOG (edit_list_dialog)->action_area;
	gtk_widget_show (dialog_action_area);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
	
	cancelbutton = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (edit_list_dialog), cancelbutton, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);
	
	applybutton = gtk_button_new_from_stock ("gtk-apply");
	gtk_widget_show (applybutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (edit_list_dialog), applybutton, GTK_RESPONSE_APPLY);
	GTK_WIDGET_SET_FLAGS (applybutton, GTK_CAN_DEFAULT);
	
	okbutton = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (edit_list_dialog), okbutton, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);
	
	data->fb = fb;
	data->store = items_store;
	data->widget = edit_list_dialog;
	
	g_signal_connect ((gpointer) edit_list_dialog, "close",
					G_CALLBACK (on_fb_edit_list_dialog_close),
					data);
	g_signal_connect ((gpointer) treeview, "row_activated",
					G_CALLBACK (on_fb_edit_list_treeview_row_activated),
					NULL);
	g_signal_connect ((gpointer) button_add, "clicked",
					G_CALLBACK (on_fb_edit_list_button_add_clicked),
					treeview);
	g_signal_connect ((gpointer) button_remove, "clicked",
					G_CALLBACK (on_fb_edit_list_button_remove_clicked),
					treeview);
	g_signal_connect ((gpointer) button_up, "clicked",
					G_CALLBACK (on_fb_edit_list_button_up_clicked),
					treeview);
	g_signal_connect ((gpointer) button_down, "clicked",
					G_CALLBACK (on_fb_edit_list_button_down_clicked),
					treeview );
	g_signal_connect ((gpointer) button_top, "clicked",
					G_CALLBACK (on_fb_edit_list_button_top_clicked),
					treeview);
	g_signal_connect ((gpointer) button_bottom, "clicked",
					G_CALLBACK (on_fb_edit_list_button_bottom_clicked),
					treeview);
	g_signal_connect ((gpointer) cancelbutton, "clicked",
					G_CALLBACK (on_fb_edit_list_cancelbutton_clicked),
					data);
	g_signal_connect ((gpointer) applybutton, "clicked",
					G_CALLBACK (on_fb_edit_list_applybutton_clicked),
					data);
	g_signal_connect ((gpointer) okbutton, "clicked",
					G_CALLBACK (on_fb_edit_list_okbutton_clicked),
					data);
	
	fb_edit_list_populate_store ( fb, items_store );
	
	return edit_list_dialog;
}

