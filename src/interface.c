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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <plugin.h>

#include "filebrowser.h"
#include "callbacks.h"
#include "interface.h"

// inline images
#include "file.h" // file.png
#include "dir.h"  // dir.png

GtkWidget*
create_ui ( CssedFileBrowser* fb )
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *combo_current;
  GtkWidget *combo_entry_current;
  GtkWidget *button_go;
  GtkWidget *alignment_go;
  GtkWidget *hbox_up;
  GtkWidget *label_go;
  GtkWidget *vpaned;
  GtkWidget *scrolledwindow_dirs;
  GtkWidget *treeview_dirs;
  GtkWidget *scrolledwindow_files;
  GtkWidget *treeview_files;
  GtkWidget *hbox_filter;
  GtkWidget *hbox_buttons;
  GtkWidget *checkbox_hidden;
  GtkWidget *button_up;
  GtkWidget *button_home;
  GtkWidget *button_fromdoc; 
  GtkWidget *button_refresh;
  GtkWidget *image;
  GtkWidget *button_index;
  GtkWidget *label_pattern;
  GtkWidget *entry_pattern;
  GtkWidget *button_clear_pattern;
  // stores and cols to use in the treeviews
  GtkListStore* directories_store;
  GtkListStore* files_store;
  GtkCellRenderer* text_renderer;
  GtkCellRenderer* image_renderer;
  GtkTreeViewColumn* dirs_image_col;
  GtkTreeViewColumn* dirs_text_col;
  GtkTreeViewColumn* files_image_col;
  GtkTreeViewColumn* files_text_col;
  // images
  GdkPixbuf* file_img;
  GdkPixbuf* dir_img;
  GtkTooltips *button_bar_tips;
  static const GtkTargetEntry targets[] = {{ "STRING", 0, 0 },{ "text/plain", 0, 0 },{ "text/uri-list", 0, 0 }};

  button_bar_tips = gtk_tooltips_new ();

  file_img = gdk_pixbuf_new_from_inline (-1, file_pixbuf, FALSE, NULL);
  dir_img = gdk_pixbuf_new_from_inline (-1, dir_pixbuf, FALSE, NULL); 

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  combo_current = gtk_combo_new ();
  //g_object_set_data (G_OBJECT (GTK_COMBO (combo_current)->popwin),
  //                   "GladeParentKey", combo_current);
  gtk_widget_show (combo_current);
  gtk_box_pack_start (GTK_BOX (hbox), combo_current, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (combo_current), 8);

  combo_entry_current = GTK_COMBO (combo_current)->entry;
  gtk_widget_show (combo_entry_current);

  button_go = gtk_button_new ();
  gtk_widget_show (button_go);
  gtk_box_pack_start (GTK_BOX (hbox), button_go, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button_go), 5);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_go,
				 _("Go to the selected directory"),
				_("Go to the selected directory"));

  alignment_go = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment_go);
  gtk_container_add (GTK_CONTAINER (button_go), alignment_go);

  hbox_up = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox_up);
  gtk_container_add (GTK_CONTAINER (alignment_go), hbox_up);

  image = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox_up), image, FALSE, FALSE, 0);

  label_go = gtk_label_new_with_mnemonic (_("Go"));
  gtk_widget_show (label_go);
  gtk_box_pack_start (GTK_BOX (hbox_up), label_go, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label_go), GTK_JUSTIFY_LEFT);

  vpaned = gtk_vpaned_new ();
  gtk_widget_show (vpaned);
  gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 0);
  gtk_paned_set_position (GTK_PANED (vpaned), 300);
  gtk_widget_set_size_request (GTK_WIDGET(vpaned), 20, 20);                                             
                                             
  scrolledwindow_dirs = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_dirs);
  gtk_paned_pack1 (GTK_PANED (vpaned), scrolledwindow_dirs, FALSE, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow_dirs), 2);

  // ------- Directories treeview ---------------------------------------------
  treeview_dirs = gtk_tree_view_new ();
  gtk_widget_show (treeview_dirs);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_dirs), treeview_dirs);

  text_renderer = gtk_cell_renderer_text_new();
  image_renderer = gtk_cell_renderer_pixbuf_new();

  directories_store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
  

  dirs_image_col = gtk_tree_view_column_new_with_attributes ("",
                                                      image_renderer,
                                                      "pixbuf", 0,
                                                       NULL);

  dirs_text_col = gtk_tree_view_column_new_with_attributes (_("Directories"),
                                                      text_renderer,
                                                      "text", 1,
                                                       NULL);

  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(directories_store), 1, GTK_SORT_ASCENDING);

  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview_dirs), GTK_TREE_MODEL (directories_store));
  g_object_unref(directories_store);
  gtk_tree_view_insert_column ( GTK_TREE_VIEW(treeview_dirs), dirs_image_col, 0);
  gtk_tree_view_insert_column ( GTK_TREE_VIEW(treeview_dirs), dirs_text_col, 1);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_dirs),  TRUE);
  gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW(treeview_dirs), TRUE);
  // -------- Scrolled window --------------------------------------------

  scrolledwindow_files = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_files);
  gtk_paned_pack2 (GTK_PANED (vpaned), scrolledwindow_files, TRUE, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow_files), 2);

  // ------- Files treeview -----------------------------------------
  treeview_files = gtk_tree_view_new ();
  gtk_widget_show (treeview_files);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_files), treeview_files);

  files_store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

  files_image_col = gtk_tree_view_column_new_with_attributes ("",
                                                      image_renderer,
                                                      "pixbuf", 0,
                                                       NULL);

  files_text_col = gtk_tree_view_column_new_with_attributes (_("Files"),
                                                      text_renderer,
                                                      "text", 1,
                                                       NULL);

  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(files_store), 1, GTK_SORT_ASCENDING);

  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview_files), GTK_TREE_MODEL (files_store));
  g_object_unref(files_store);
  gtk_tree_view_insert_column ( GTK_TREE_VIEW(treeview_files), files_image_col, 0 );
  gtk_tree_view_insert_column ( GTK_TREE_VIEW(treeview_files), files_text_col, 1 );

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_files),  TRUE);
  gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW(treeview_files), TRUE);

  // ------------- Drag'n'Drop ------------------------------------------------
	gtk_drag_source_set (GTK_WIDGET (treeview_files),
		       GDK_BUTTON1_MASK,
		       targets,
		       3,
		       GDK_ACTION_COPY|GDK_ACTION_MOVE);
	gtk_drag_source_set (GTK_WIDGET (treeview_dirs),
		       GDK_BUTTON1_MASK,
		       targets,
		       3,
		       GDK_ACTION_COPY|GDK_ACTION_MOVE);		       
  // --------------------------------------------------------------------------

	 
  hbox_filter = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_filter);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_filter, FALSE, FALSE, 5);

  label_pattern = gtk_label_new (_("Filter"));
  gtk_widget_show (label_pattern);
  gtk_box_pack_start (GTK_BOX (hbox_filter),label_pattern , FALSE, FALSE, 10);

  entry_pattern = gtk_entry_new();
  gtk_widget_show (entry_pattern);
  gtk_box_pack_start (GTK_BOX (hbox_filter),entry_pattern , TRUE, TRUE, 0);
  
  button_clear_pattern= gtk_button_new ();
  gtk_widget_show (button_clear_pattern);
  gtk_box_pack_start (GTK_BOX (hbox_filter), button_clear_pattern, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_clear_pattern,
				 _("Clear the filter"),
				_("This button will clear the filter in any"));

  image = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_clear_pattern), image);


  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), entry_pattern,
				 _("Enter a pattern to filter as *.css"),
				_("Enter a pattern to filter, it support globbing as *.css or single character matching as ?akefile. You cannot escape globs to include them"));

  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_buttons);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_buttons, FALSE, FALSE, 5);
  
  checkbox_hidden = gtk_check_button_new_with_label (_("Show hidden"));
  gtk_widget_show (checkbox_hidden);
  gtk_box_pack_start (GTK_BOX (hbox_buttons),checkbox_hidden , TRUE, TRUE, 0);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), checkbox_hidden,
				 _("Show hidden files"),
				 _("When toggled you will be able to see the files hidden in the directory"));


  button_fromdoc = gtk_button_new ();
  gtk_widget_show (button_fromdoc);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_fromdoc, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_fromdoc,
				 _("Go to current document's directory"),
				 _("With this button you can move to the current document's directory"));

  image = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_fromdoc), image);
  
  button_index = gtk_button_new ();
  gtk_widget_show (button_index);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_index, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_index,
				 _("Edit the drop down list"),
				_("This button will show a dialog to let you edit the items in the upper drop down list (last opened directories history)"));

  image = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_index), image);	



  button_up = gtk_button_new ();
  gtk_widget_show (button_up);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_up, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_up,
				 _("Go up one directory"),
				 _("With this button you can move to the parent directory if any"));

  image = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_up), image);
  
  button_refresh = gtk_button_new ();
  gtk_widget_show (button_refresh);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_refresh, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_refresh,
				 _("Refresh the current directory"),
				_("This button let you refresh - meaning re-read - the current directory"));

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_refresh), image);
  
  button_home = gtk_button_new ();
  gtk_widget_show (button_home);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_home, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button_home,
				 _("Go to home"),
				 _("With this button you can move to your home directory"));

  image = gtk_image_new_from_stock ("gtk-home", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button_home), image);

  fb->entry_directory = combo_entry_current ;
  fb->dir_store = directories_store;
  fb->file_store = files_store;
  fb->treeview_files = treeview_files;
  fb->treeview_directories = treeview_dirs;
  fb->current_dir = g_strdup(  g_get_home_dir () );
  fb->combo_directory = combo_current;
  fb->dir_img = dir_img;
  fb->file_img = file_img;
  fb->checkbox_hidden = checkbox_hidden;
  fb->entry_pattern = entry_pattern;
  fb->button_clear_pattern = button_clear_pattern;

  g_signal_connect ((gpointer) combo_entry_current,"key-press-event" ,
                    G_CALLBACK (on_fb_combo_entry_current_key_press_event),
                    fb);
  g_signal_connect ((gpointer) entry_pattern,"key-press-event" ,
                    G_CALLBACK (on_fb_combo_entry_pattern_key_press_event),
                    fb);
  g_signal_connect ((gpointer) button_clear_pattern, "clicked",
                    G_CALLBACK (on_fb_button_clear_pattern_clicked),
                    fb);
  g_signal_connect ((gpointer) button_go, "clicked",
                    G_CALLBACK (on_fb_button_go_clicked),
                    fb);
  g_signal_connect ((gpointer) treeview_dirs, "row_activated",
                    G_CALLBACK (on_fb_treeview_dirs_row_activated),
                    fb);
  g_signal_connect ((gpointer) treeview_files, "row_activated",
                    G_CALLBACK (on_fb_treeview_files_row_activated),
                    fb);                    
  g_signal_connect ((gpointer) treeview_files, "drag-data-get",
                    G_CALLBACK (on_fb_treeview_files_drag_data_get),
                    fb);
  g_signal_connect ((gpointer) treeview_dirs, "drag-data-get",
                    G_CALLBACK (on_fb_treeview_files_drag_data_get),
                    fb);                                        
  g_signal_connect ((gpointer) treeview_files, "button-press-event",
                    G_CALLBACK (on_fb_treeview_button_press_event),
                    fb);                                       
  g_signal_connect ((gpointer) treeview_dirs, "button-press-event",
                    G_CALLBACK (on_fb_treeview_button_press_event),
                    fb);
  g_signal_connect ((gpointer) button_index, "clicked",
                    G_CALLBACK (on_fb_button_index_clicked),
                    fb);
  g_signal_connect ((gpointer) button_refresh, "clicked",
                    G_CALLBACK (on_fb_button_refresh_clicked),
                    fb);
  g_signal_connect ((gpointer) button_up, "clicked",
                    G_CALLBACK (on_fb_button_up_clicked),
                    fb);
  g_signal_connect ((gpointer) button_home, "clicked",
                    G_CALLBACK (on_fb_button_home_clicked),
                    fb);
  g_signal_connect ((gpointer) button_fromdoc, "clicked",
                    G_CALLBACK (on_fb_button_fromdoc_clicked),
                    fb);


  return vbox;
}

#ifndef WIN32
GtkWidget*
create_dialog_unix_file_perms (FilePermsUI *ui)
{
  GtkWidget *dialog_unix_file_perms;
  GtkWidget *dialog_vbox;
  GtkWidget *scrolledwindow;
  GtkWidget *viewport;
  GtkWidget *vbox;
  GtkWidget *table_perms;
  GtkWidget *label_owner;
  GtkWidget *label_group;
  GtkWidget *label_others;
  GtkWidget *checkbutton_owner_read;
  GtkWidget *checkbutton_group_read;
  GtkWidget *checkbutton_others_read;
  GtkWidget *checkbutton_owner_write;
  GtkWidget *checkbutton_group_write;
  GtkWidget *checkbutton_others_write;
  GtkWidget *checkbutton_owner_execute;
  GtkWidget *checkbutton_group_execute;
  GtkWidget *checkbutton_others_execute;
  GtkWidget *hseparator;
  GtkWidget *hbox_flags;
  GtkWidget *checkbutton_setuid;
  GtkWidget *checkbutton_setgid;
  GtkWidget *checkbutton_sticky_bit;
  GtkWidget *dialog_action_area;
  GtkWidget *cancelbutton;
  GtkWidget *okbutton;

  dialog_unix_file_perms = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog_unix_file_perms), _("File permisions"));
  gtk_window_set_modal (GTK_WINDOW (dialog_unix_file_perms), TRUE);

  gtk_window_set_default_size (GTK_WINDOW(dialog_unix_file_perms), 300, 200);

  dialog_vbox = GTK_DIALOG (dialog_unix_file_perms)->vbox;
  gtk_widget_show (dialog_vbox);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (dialog_vbox), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  viewport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), viewport);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (viewport), vbox);

  table_perms = gtk_table_new (3, 4, TRUE);
  gtk_widget_show (table_perms);
  gtk_box_pack_start (GTK_BOX (vbox), table_perms, TRUE, TRUE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (table_perms), 4);

  label_owner = gtk_label_new (_("<b>Owner:</b>"));
  gtk_widget_show (label_owner);
  gtk_table_attach (GTK_TABLE (table_perms), label_owner, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label_owner), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_owner), 0, 0.5);

  label_group = gtk_label_new (_("<b>Group:</b>"));
  gtk_widget_show (label_group);
  gtk_table_attach (GTK_TABLE (table_perms), label_group, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label_group), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_group), 0, 0.5);

  label_others = gtk_label_new (_("<b>Others:</b>"));
  gtk_widget_show (label_others);
  gtk_table_attach (GTK_TABLE (table_perms), label_others, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label_others), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label_others), 0, 0.5);

  checkbutton_owner_read = gtk_check_button_new_with_mnemonic (_("Read"));
  gtk_widget_show (checkbutton_owner_read);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_owner_read, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_group_read = gtk_check_button_new_with_mnemonic (_("Read"));
  gtk_widget_show (checkbutton_group_read);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_group_read, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_others_read = gtk_check_button_new_with_mnemonic (_("Read"));
  gtk_widget_show (checkbutton_others_read);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_others_read, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_owner_write = gtk_check_button_new_with_mnemonic (_("Write"));
  gtk_widget_show (checkbutton_owner_write);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_owner_write, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_group_write = gtk_check_button_new_with_mnemonic (_("Write"));
  gtk_widget_show (checkbutton_group_write);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_group_write, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_others_write = gtk_check_button_new_with_mnemonic (_("Write"));
  gtk_widget_show (checkbutton_others_write);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_others_write, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_owner_execute = gtk_check_button_new_with_mnemonic (_("Execute"));
  gtk_widget_show (checkbutton_owner_execute);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_owner_execute, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_group_execute = gtk_check_button_new_with_mnemonic (_("Execute"));
  gtk_widget_show (checkbutton_group_execute);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_group_execute, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbutton_others_execute = gtk_check_button_new_with_mnemonic (_("Execute"));
  gtk_widget_show (checkbutton_others_execute);
  gtk_table_attach (GTK_TABLE (table_perms), checkbutton_others_execute, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 0);

  hbox_flags = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox_flags);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_flags, TRUE, TRUE, 0);

  checkbutton_setuid = gtk_check_button_new_with_mnemonic (_("Set user ID"));
  gtk_widget_show (checkbutton_setuid);
  gtk_box_pack_start (GTK_BOX (hbox_flags), checkbutton_setuid, FALSE, FALSE, 0);

  checkbutton_setgid = gtk_check_button_new_with_mnemonic (_("Set group ID"));
  gtk_widget_show (checkbutton_setgid);
  gtk_box_pack_start (GTK_BOX (hbox_flags), checkbutton_setgid, FALSE, FALSE, 0);

  checkbutton_sticky_bit = gtk_check_button_new_with_mnemonic (_("Sticky"));
  gtk_widget_show (checkbutton_sticky_bit);
  gtk_box_pack_start (GTK_BOX (hbox_flags), checkbutton_sticky_bit, FALSE, FALSE, 0);

  dialog_action_area = GTK_DIALOG (dialog_unix_file_perms)->action_area;
  gtk_widget_show (dialog_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

  cancelbutton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog_unix_file_perms), cancelbutton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);

  okbutton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog_unix_file_perms), okbutton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);

  ui->checkbutton_owner_read = checkbutton_owner_read;
  ui->checkbutton_group_read = checkbutton_group_read;
  ui->checkbutton_others_read = checkbutton_others_read;
  ui->checkbutton_owner_write = checkbutton_owner_write;
  ui->checkbutton_group_write = checkbutton_group_write;
  ui->checkbutton_others_write = checkbutton_others_write;
  ui->checkbutton_owner_execute = checkbutton_owner_execute;
  ui->checkbutton_group_execute = checkbutton_group_execute;
  ui->checkbutton_others_execute = checkbutton_others_execute;
  ui->checkbutton_setuid = checkbutton_setuid;
  ui->checkbutton_setgid = checkbutton_setgid;
  ui->checkbutton_sticky_bit = checkbutton_sticky_bit;

  return dialog_unix_file_perms;
}
#endif




