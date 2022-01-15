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

#include <gtk/gtk.h>


gboolean
on_fb_combo_entry_current_key_press_event (GtkWidget *widget,
                                            GdkEventKey *event,
                                            gpointer user_data);
gboolean
on_fb_combo_entry_pattern_key_press_event (GtkWidget *widget,
                                            GdkEventKey *event,
                                            gpointer user_data);
void
on_fb_button_go_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_fb_treeview_dirs_row_activated      (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_fb_treeview_files_row_activated        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);
void
on_fb_button_index_clicked				(GtkButton       *button,
                                        gpointer         user_data);
void
on_fb_button_refresh_clicked				(GtkButton       *button,
                                        gpointer         user_data);

void
on_fb_button_up_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_fb_button_home_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_fb_button_fromdoc_clicked               (GtkButton       *button,
                                        gpointer         user_data);
                                        
void
on_fb_button_clear_pattern_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
fb_scan_directory         ( gchar* dirname,
							CssedFileBrowser* fb ,
							gboolean show_hidden );


void
on_fb_treeview_files_drag_data_get 		( GtkWidget *widget,
                                            GdkDragContext *drag_context,
                                            GtkSelectionData *data,
                                            guint info,
                                            guint time,
                                            gpointer user_data);

gboolean
on_fb_treeview_button_press_event (GtkWidget *treeview, 
										GdkEventButton *event,
										gpointer userdata);

void
fb_save_list_to_file( CssedFileBrowser* fb );

void
fb_load_list_from_file( CssedFileBrowser* fb );


gboolean
fb_load_homedir_at_idle ( gpointer fb );
