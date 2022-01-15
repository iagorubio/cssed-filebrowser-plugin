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
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef  WIN32
# include <unistd.h>
#else
# include <direct.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <plugin.h>

#include "filebrowser.h"
#include "callbacks.h"
#include "interface.h"
#include "editlist.h"
#include "file.h"


#ifndef MAX_PATH
# define MAX_PATH 1024
#endif

#define FB_CONFIG_FILE "/.cssed/filebrowser"

// returned value must be freed when not needed
gchar*
path_up_one_directory( gchar* path )
{
  gchar** comp;
  gchar* new_path;
  gchar* old_path;
  gint len=0,i;

  if( !g_path_is_absolute (path) ){
    // FIXME warn users to avoid to use relative paths
    return g_strdup( path );
  }

#ifdef WIN32
  if( strlen( path ) < 4 ) //drive letter
		return g_strdup( path );
#endif

  if( (strlen( path ) > 1) && (path[ strlen(path) - 1 ] == G_DIR_SEPARATOR) ){
    path[ strlen(path) - 1 ] = '\0';
  }
  
  comp = g_strsplit (path, G_DIR_SEPARATOR_S , 0);
  len = g_strv_length (comp);

  if( --len == 1 ){
#ifndef WIN32
    new_path = g_strdup(G_DIR_SEPARATOR_S);
#else
    new_path = g_strconcat(comp[0], G_DIR_SEPARATOR_S, NULL);
#endif
  }else{
    for( i=0; i<len;i++ ){
      if( i == 0 ){
 #ifndef WIN32
		new_path = g_strdup_printf("%s%s", G_DIR_SEPARATOR_S, comp[i]);
#else
		new_path = g_strdup_printf("%s", comp[i]);
#endif
      }else{
        old_path = new_path;
		new_path = g_build_filename(old_path, comp[i], NULL);
        g_free(old_path);
      }
    }
  }

  g_strfreev(comp);

  return new_path;
}


gboolean
on_fb_combo_entry_current_key_press_event (GtkWidget *widget,
                                           GdkEventKey *event,
                                           gpointer user_data)
{
  CssedFileBrowser* fb;
  gchar* dir;

  fb = (CssedFileBrowser*) user_data;

  if( event->keyval == GDK_Return ){
    dir = gtk_editable_get_chars(GTK_EDITABLE(fb->entry_directory),0,-1);
    fb_scan_directory (dir, fb, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(fb->checkbox_hidden)));
    g_free(dir);
    return TRUE;
  }  
  return FALSE;
}

gboolean
on_fb_combo_entry_pattern_key_press_event  (GtkWidget *widget,
                                         GdkEventKey *event,
                                         gpointer user_data)
{
	if( event->keyval == GDK_Return ){
		// wrap to the button clicked func
		on_fb_button_go_clicked (NULL, user_data);
	}
   return FALSE;
}

void
on_fb_button_go_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  CssedFileBrowser* fb;
  gchar* dir;

  fb = (CssedFileBrowser*) user_data;

  dir = gtk_editable_get_chars(GTK_EDITABLE( fb->entry_directory ), 0, -1);
  //path_up_one_directory(dir);
  fb_scan_directory (dir, fb, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(fb->checkbox_hidden)));
  g_free(dir);
}


void
on_fb_treeview_dirs_row_activated         (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	CssedFileBrowser* fb;
	GtkTreeIter iter;
	gchar* directory;
	gchar* new_path;
	
	fb = (CssedFileBrowser*) user_data;
	
	if (gtk_tree_model_get_iter (GTK_TREE_MODEL(fb->dir_store), &iter, path))
	{
		gtk_tree_model_get (GTK_TREE_MODEL(fb->dir_store), &iter, 1, &directory, -1);
		new_path = g_build_filename( fb->current_dir, directory, NULL );
		//new_path = g_strdup_printf("%s/%s", fb->current_dir, directory );
		fb_scan_directory    ( new_path, fb, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)) );
		g_free(new_path);
		g_free(directory);
	} 
}

void
on_fb_treeview_files_row_activated        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
  CssedFileBrowser* fb;
  GtkTreeIter iter;
  gchar* filename;
  gchar* fullpath;
  GList* open_list;

  fb = (CssedFileBrowser*) user_data;

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL(fb->file_store), &iter, path))
	{
		gtk_tree_model_get (GTK_TREE_MODEL(fb->file_store), &iter, 1, &filename, -1);
		fullpath = g_build_filename ( fb->current_dir , filename, NULL );
		//fullpath = g_strdup_printf("%s/%s", fb->current_dir , filename );
		cssed_plugin_open_file(fb->plugin, fullpath );
		g_free( fullpath );
		g_free( filename );			
		open_list = g_list_first(fb->dirlist);
		while( open_list ){
			if( strcmp((gchar*) open_list->data, fb->current_dir) == 0 )
				return;
			else
				open_list = g_list_next(open_list);
		}
		fb->dirlist = g_list_append (fb->dirlist, g_strdup(fb->current_dir));
		gtk_combo_set_popdown_strings (GTK_COMBO(fb->combo_directory), fb->dirlist);	
	} 
}

void
on_fb_button_index_clicked				(GtkButton       *button,
                                        gpointer         user_data)
{
	CssedFileBrowser* fb;	
	GtkWidget* dialog;

	fb = (CssedFileBrowser*) user_data;
	dialog = fb_create_edit_list_dialog (fb);
	gtk_widget_show(dialog);
}


void
on_fb_button_refresh_clicked	(GtkButton       *button,
                                 gpointer         user_data)
{
	CssedFileBrowser* fb;	
	fb = (CssedFileBrowser*) user_data;
	fb_scan_directory (NULL, 
						fb, 
						gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden))
					);
}

  void
on_fb_button_up_clicked   (GtkButton       *button,
                           gpointer         user_data)
{
	CssedFileBrowser* fb;
	gchar* basedir;

	fb = (CssedFileBrowser*) user_data;	
	basedir = path_up_one_directory( fb->current_dir );	
	fb_scan_directory (basedir, fb, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)));
	g_free( basedir );
}


  void
on_fb_button_fromdoc_clicked   (GtkButton       *button,
                             gpointer         user_data)
{
	CssedFileBrowser *fb;
	CssedPlugin *plugin;
	gchar *dirname, *filename;
	
	fb = (CssedFileBrowser*) user_data;		
	plugin = (CssedPlugin*) fb->plugin;
	
	filename = cssed_plugin_document_get_filename (plugin);
	if( filename ){
		dirname = g_path_get_dirname( filename );
		fb_scan_directory (dirname, fb, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)));
		g_free(dirname);
		g_free(filename);
	}else{
		cssed_plugin_error_message(_("File not saved"), _("Cannot get the current document's directory\nthis file is not saved on disk"));
	}
}


void
on_fb_button_home_clicked   (GtkButton       *button,
                             gpointer         user_data)
{
	CssedFileBrowser* fb;

	fb = (CssedFileBrowser*) user_data;	
	fb_scan_directory ((gchar*) g_get_home_dir (), fb, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)));
}

void
on_fb_button_clear_pattern_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* entry;
	CssedFileBrowser* fb;

	fb = (CssedFileBrowser*) user_data;	
	entry = fb->entry_pattern ;
	gtk_entry_set_text (GTK_ENTRY(entry), "");	
	on_fb_button_refresh_clicked (NULL,  user_data);
}


void
fb_scan_directory    ( gchar* dirname, CssedFileBrowser* fb, gboolean show_hidden )
{
 	GtkListStore* dir_store; 
	GtkListStore* file_store;
	GtkWidget* direntry;
	GtkWidget* entry_pattern;
	GdkPixbuf* file_img;
	GdkPixbuf* dir_img;    
	GDir *dir;
	GError *error = NULL;
	G_CONST_RETURN gchar *filename;
	GtkTreeIter iter;
	gchar* old_path;
	gchar* pattern = NULL;
	gchar* utf8_name;
	gchar *fullname;
	
	dir_store = fb->dir_store; 
	file_store = fb->file_store;
	direntry = fb->entry_directory;
	file_img = fb->file_img;
	dir_img = fb->dir_img; 
	entry_pattern = fb->entry_pattern;

	pattern = gtk_editable_get_chars(GTK_EDITABLE(entry_pattern), 0, -1);	
	// pass NULL to refresh
	if( dirname == NULL ){
		dirname = fb->current_dir;
	}

	dir = g_dir_open (dirname, 0, &error);
	
	if (dir != NULL)
	{
		gtk_entry_set_text(GTK_ENTRY(direntry), dirname);
		// if the directory did not change we're refreshing
		//  and there's no need to update fb->current_dir
		if( strcmp(dirname, fb->current_dir) != 0){
			old_path = fb->current_dir;
			fb->current_dir = g_strdup(dirname);
			g_free(old_path);
		}		
		// clear old data
		gtk_list_store_clear (dir_store);
		gtk_list_store_clear (file_store);

		filename = g_dir_read_name (dir);
		while (filename)
		  {
			fullname = g_build_filename( dirname, filename, NULL ); 

			if ( g_file_test(fullname, G_FILE_TEST_IS_DIR))
			  {
				if( show_hidden ){
				  utf8_name = 	g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
				  gtk_list_store_append (dir_store, &iter);
				  gtk_list_store_set (dir_store, &iter, 0, dir_img, 1, utf8_name, -1 );
				  g_free(utf8_name);				
				}else{
				  if( filename[0]!='.' ){
				  utf8_name = 	g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
					gtk_list_store_append (dir_store, &iter);
					gtk_list_store_set (dir_store, &iter, 0, dir_img, 1, utf8_name, -1 );
				    g_free(utf8_name);                
				  }            
				}
			  }
			else if( g_file_test(fullname, G_FILE_TEST_IS_REGULAR) )
			  {
				if( (pattern != NULL) && (strlen(pattern) > 0) ){
					if( g_pattern_match_simple(pattern, filename) )
					{
						if( show_hidden ){
						  utf8_name = 	g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
						  gtk_list_store_append (file_store,   &iter);
						  gtk_list_store_set (file_store, &iter, 0, file_img, 1,  utf8_name, -1 );
						  g_free(utf8_name);						
						}else{
						  if( filename[0]!='.' ){
							  utf8_name = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
							  gtk_list_store_append (file_store,   &iter);
							  gtk_list_store_set (file_store, &iter, 0, file_img, 1, utf8_name, -1 );
							  g_free(utf8_name);						
							}
						}
					}
				}else{
					if( show_hidden ){
					  utf8_name = 	g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
					  gtk_list_store_append (file_store,   &iter);
					  gtk_list_store_set (file_store, &iter, 0, file_img, 1, utf8_name, -1 );
					  g_free(utf8_name);						
					}else{
					  if( filename[0]!='.' ){
					    utf8_name = 	g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
					    gtk_list_store_append (file_store,   &iter);
						gtk_list_store_set (file_store, &iter, 0, file_img, 1, utf8_name, -1 );
					    g_free(utf8_name);
					  }
					}
				}
			  }
			g_free (fullname);
			filename = g_dir_read_name (dir);
		  }
		g_dir_close (dir);
	}else{
		cssed_plugin_error_message(_("Cannot open file"), error->message);
		g_error_free(error);
	}

	if( pattern )  g_free(pattern);
}

void
on_fb_treeview_files_drag_data_get 		  ( GtkWidget *widget,
                                            GdkDragContext *drag_context,
                                            GtkSelectionData *data,
                                            guint info,
                                            guint time,
                                            gpointer user_data )
{
	CssedFileBrowser* fb;
	CssedPlugin* plugin;
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	gchar* path;
	gchar* name;

	fb = (CssedFileBrowser*) user_data;
	plugin = fb->plugin;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &name, -1);
		path = g_build_filename( fb->current_dir, name, NULL );
		//path =  g_strdup_printf("%s/%s",fb->current_dir ,name  );
		gtk_selection_data_set(
				data,
				GDK_SELECTION_TYPE_STRING,
				8,
				(const guchar*) path, 
				strlen(path)
			);
		g_free(path);
		g_free(name);
	}

}
/* contextual menu in files treeview, adds delete */
  void
fb_treeview_delete_selection	(GtkWidget *item, gpointer user_data)
{
	GtkWidget* treeview;
	CssedFileBrowser* fb;
	gchar* curdir;
	gchar* file_name;
	gchar* full_path;
	GtkTreeSelection *selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkWidget* confirm;
	extern int errno;
	gint response;
	
	fb = (CssedFileBrowser*) user_data;
	treeview = fb->treeview_files;
	curdir = fb->current_dir;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if( !gtk_tree_selection_get_selected (selection, &model, &iter) ) return;
	gtk_tree_model_get(model, &iter, 1, &file_name, -1);
	
	full_path = g_build_filename( curdir, file_name, NULL );

	confirm = gtk_message_dialog_new        (NULL,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_QUESTION,
                                             GTK_BUTTONS_YES_NO,
                                             _("Are you sure you want to delete\n%s ?"),
											 full_path);
	response = gtk_dialog_run(GTK_DIALOG(confirm));
	gtk_widget_destroy(confirm);
	
	if( response == GTK_RESPONSE_YES ){
		if( g_file_test(full_path, G_FILE_TEST_EXISTS) ){
			if( unlink( full_path ) != -1 ){
				fb_scan_directory ( NULL, // with NULL directory it'll refresh current one
									fb, 
									gtk_toggle_button_get_active(
										GTK_TOGGLE_BUTTON(fb->checkbox_hidden)
									));
			}else{
				cssed_plugin_error_message(_("Can not delete file"), strerror(errno));
			}
		}else{
			cssed_plugin_error_message( _("Can not delete file"), _("File does not exists on disk,\nplease refresh the view."));
		}
	}
	g_free(full_path);
}

void
fb_treeview_copy_selection	(GtkWidget *treeview, gpointer user_data)
{
	CssedFileBrowser* fb;
	gchar* curdir;
	gchar* file_name;
	gchar* full_path;
	GtkTreeSelection *selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkClipboard* clipboard;

	fb = (CssedFileBrowser*) user_data;
	curdir = fb->current_dir;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if( !gtk_tree_selection_get_selected (selection, &model, &iter) ) return;
	
	gtk_tree_model_get(model, &iter, 1, &file_name, -1);	
	full_path = g_build_filename( curdir, file_name, NULL );

	clipboard = gtk_clipboard_get (GDK_NONE);
	gtk_clipboard_set_text( clipboard,
							full_path,
							strlen(full_path) );
	g_free(full_path);
}

void
fb_treeview_copy_directory	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_copy_selection	(fb->treeview_directories, user_data);
}

void
fb_treeview_copy_file	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_copy_selection	(fb->treeview_files, user_data);
}

void
fb_treeview_rename_selection	(GtkWidget *treeview, gpointer user_data)
{
	CssedFileBrowser* fb;
	CssedPlugin *plugin;
	gchar* curdir;
	gchar* file_name;
	gchar* full_path;
	gchar* new_path = NULL;
	gchar *err_msg, *cannot_msg_hdr, *cannot_msg;
	GtkTreeSelection *selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	extern int errno;

	fb = (CssedFileBrowser*) user_data;
	curdir = fb->current_dir;
	plugin = fb->plugin;	
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if( !gtk_tree_selection_get_selected (selection, &model, &iter) ) return;
	gtk_tree_model_get(model, &iter, 1, &file_name, -1);
			
	if( treeview == fb->treeview_files ){
		full_path = g_build_filename( curdir, file_name, NULL);
		new_path = cssed_plugin_prompt_for_file_to_save(plugin,_("Enter new file name"), full_path);
		err_msg = _("Error renaming file");
		cannot_msg_hdr =  _("Cannot rename file");
		cannot_msg = _("File does not exists on disk,\nplease refresh the view.");
	}else if( treeview == fb->treeview_directories ){
		new_path = cssed_plugin_prompt_for_directory_to_create(plugin,_("Enter new directory name"), curdir);
		full_path = g_build_filename( curdir, file_name, NULL);
		err_msg = _("Error renaming file directory");
		cannot_msg_hdr =  _("Cannot rename directory");
		cannot_msg = _("Directory does not exists on disk,\nplease refresh the view.");
	}else{
		g_warning("** bogus treeview passed to fb_treeview_rename_selection");
	}
	
	if( new_path == NULL ) {// user cancel or error
		g_free(file_name);
		g_free(full_path);	
		return; 
	}
	
	if( g_file_test(full_path, G_FILE_TEST_EXISTS) ){
		if( g_file_test(new_path, G_FILE_TEST_EXISTS ) ){
			if( !cssed_plugin_confirm_dialog  	( 
												  _("Renaming: destination file already exists"),
												  _("The destination file/directory already exists on disk:\n%s\n\nIf you use its name, you will overwrite it and it will be lost.\n\nIs all right to continue and loose %s ?\n\nClick \"Cancel\" to avoid to loose %s."), 
												  new_path,
												  new_path,
												  new_path
												)
			  ){
				g_free(file_name);
				g_free(full_path);
				g_free(new_path);
				
				return;
			}
		}
		
		if( rename( full_path, new_path ) != -1 ){
			fb_scan_directory ( NULL, // with NULL directory it'll refresh current one
								fb, 
								gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)) );
		}else{
			cssed_plugin_error_message( err_msg, strerror(errno) );
		}
	}else{
		cssed_plugin_error_message(cannot_msg_hdr, cannot_msg);
	}
	
	g_free(file_name);
	g_free(full_path);
	g_free(new_path);
}

void
fb_treeview_rename_directory	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_rename_selection	(fb->treeview_directories, user_data);
}

void
fb_treeview_rename_file	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_rename_selection	(fb->treeview_files, user_data);
}

#ifndef WIN32
void
fb_treeview_change_perms_selection	(GtkWidget *treeview, gpointer user_data)
{
	CssedFileBrowser *fb;
	gchar *curdir;
	gchar *file_name;
	gchar *full_path;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct stat buf;
	mode_t  mode;
	FilePermsUI *ui;
	GtkWidget *dialog;
	gint *flags;
	gint i = 0;
	gint x = 0;
	gint perms;

	fb = (CssedFileBrowser*) user_data;
	curdir = fb->current_dir;
	// a selection should exists, it's ensured by 
	// on_fb_treeview_button_press_event
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if( !gtk_tree_selection_get_selected (selection, &model, &iter) ) return;
	gtk_tree_model_get(model, &iter, 1, &file_name, -1);
	
	full_path = g_build_filename( curdir, file_name, NULL);
	g_free(file_name);
	
	if( stat((const gchar*)full_path, &buf) == 0)
	{
		mode = buf.st_mode;		
		ui = g_malloc0(sizeof(FilePermsUI));
		dialog = create_dialog_unix_file_perms (ui);

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_setuid), mode & S_ISUID); 
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_setgid),  mode & S_ISGID); 
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_sticky_bit), mode & S_ISVTX); 

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_read), mode & S_IRUSR);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_write), mode & S_IWUSR);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_execute), mode & S_IXUSR);

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_read), mode & S_IRGRP);		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_write), mode & S_IWGRP);	
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_execute), mode & S_IXGRP);	

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_read), mode & S_IROTH);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_write), mode & S_IWOTH);	
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_execute),  mode & S_IXOTH);

		if( gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
			flags = g_new0(gint, 12);
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_setuid)) ) flags[i++] = S_ISUID;
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_setgid)) ) flags[i++] = S_ISGID; 
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_sticky_bit)) ) flags[i++] = S_ISVTX; 
	
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_read)) ) flags[i++] = S_IRUSR;
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_write)) ) flags[i++] = S_IWUSR;
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_owner_execute)) ) flags[i++] = S_IXUSR;
	
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_read)) ) flags[i++] = S_IRGRP;		
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_write)) ) flags[i++] = S_IWGRP;	
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_group_execute)) ) flags[i++] = S_IXGRP;	
	
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_read)) ) flags[i++] = S_IROTH;
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_write)) ) flags[i++] = S_IWOTH;	
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(ui->checkbutton_others_execute)) ) flags[i++] = S_IXOTH;	

			perms = flags[x];
			while( flags[++x] != 0 && x < 12 ) perms |= flags[x];

			if( chmod((const char*)full_path, perms) == -1 ){
				cssed_plugin_error_message(_("Cannot change permissions"), _("Unable to modify file permissions.\n\n%s\n"), strerror(errno));
			}			
			g_free(flags);
		}
		gtk_widget_destroy(dialog);
		g_free(ui);
	}
	else
	{
		cssed_plugin_error_message(_("Cannot get file data"),_("Unable to get file permissions.\n\n%s\n"), strerror(errno));
	}
	g_free(full_path);
}

  void
fb_treeview_change_perms_file	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_change_perms_selection	(fb->treeview_files, user_data);
}

  void
fb_treeview_change_perms_directory	(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	fb = (CssedFileBrowser*) user_data;	
	fb_treeview_change_perms_selection	(fb->treeview_directories, user_data);
}
#endif

void
fb_treeview_create_dir(GtkWidget *item, gpointer user_data)
{
	CssedFileBrowser* fb;
	CssedPlugin *plugin;
	gchar* curdir;
	gchar* new_path;
	extern int errno;

	fb = (CssedFileBrowser*) user_data;
	curdir = fb->current_dir;
	plugin = fb->plugin;	

	//full_path = g_build_filename( curdir, file_name, NULL );
	new_path = cssed_plugin_prompt_for_directory_to_create (plugin,_("Enter directory name"), curdir);

	if( new_path == NULL ) return; // user cancel
	
// Only file selector build can fail on this, 2.4 file chooser won't let you choose a bogus directory
	if( g_file_test( new_path,  G_FILE_TEST_EXISTS ) ){
		cssed_plugin_error_message(_("This file already exists"), _("This file exists on the system.\n%s\n\nPlease use other name for the new directory"), new_path);
		g_free( new_path );
		return;	
	}
		

#ifndef GTK_ATLEAST_2_6 // gtk 2.6  crates the dir by itself  
	if( g_mkdir( new_path, 0770 ) != -1 ){
		fb_scan_directory ( new_path,
							fb, 
							gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)) );
	}else{
		cssed_plugin_error_message(_("Directory creation error"), strerror(errno));
	}
#else
	fb_scan_directory ( new_path,
						fb, 
						gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fb->checkbox_hidden)) );
#endif

	g_free(new_path);
}

  void
fb_treeview_show_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
	CssedFileBrowser *fb;
	GtkWidget *menu;
	GtkWidget *menuitem;
	GtkWidget* separator;

	fb = (CssedFileBrowser*) userdata;	

	menu = gtk_menu_new();
	
	if( treeview == fb->treeview_files ){
		menuitem =  gtk_menu_item_new_with_label(_("Copy path"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						 (GCallback) fb_treeview_copy_file, userdata);	
						 
		menuitem = gtk_menu_item_new_with_label(_("Rename"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						 (GCallback) fb_treeview_rename_file, userdata);	
						 
#ifndef WIN32
		menuitem = gtk_menu_item_new_with_label(_("Change permissions"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						(GCallback) fb_treeview_change_perms_file, userdata);	
#endif		 	
		separator = gtk_menu_item_new();
		gtk_widget_set_sensitive(separator, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
		
	
		menuitem = gtk_image_menu_item_new_from_stock("gtk-delete", NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						(GCallback) fb_treeview_delete_selection, userdata);		
						 
		gtk_widget_show_all(menu);	
	}else if(treeview == fb->treeview_directories ){
		menuitem =  gtk_menu_item_new_with_label(_("Copy path"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						(GCallback) fb_treeview_copy_directory, userdata);	
	
		menuitem = gtk_menu_item_new_with_label(_("Rename"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
					    (GCallback) fb_treeview_rename_directory, userdata);	
#ifndef WIN32
		menuitem = gtk_menu_item_new_with_label(_("Change permissions"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						(GCallback) fb_treeview_change_perms_directory, userdata);	
#endif
		separator = gtk_menu_item_new();
		gtk_widget_set_sensitive(separator, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	
		menuitem = gtk_menu_item_new_with_label(_("Create directory"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect(menuitem, "activate",
						(GCallback) fb_treeview_create_dir, userdata);		
						 
		gtk_widget_show_all(menu);		
	}else{
		return;
	} 
	/*  Note: event can be NULL here
	 *  gdk_event_get_time() accepts a NULL argument */
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				   (event != NULL) ? event->button : 0,
				   gdk_event_get_time((GdkEvent*)event));
}

  gboolean
on_fb_treeview_button_press_event (GtkWidget *treeview, 
										GdkEventButton *event,
										gpointer userdata)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;

	/* single click with the right mouse button? */
	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3)
	{	
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		// Note: gtk_tree_selection_count_selected_rows() does not
		// exist in gtk+-2.0, only in gtk+ >= v2.2 !  FIXME
		if (gtk_tree_selection_count_selected_rows(selection)  <= 1)
		{
		   // Get tree path for row that was clicked 
		   if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
											 (gint) event->x, (gint) event->y,
											 &path, NULL, NULL, NULL))
		   {
			 gtk_tree_selection_unselect_all(selection);
			 gtk_tree_selection_select_path(selection, path);
			 gtk_tree_path_free(path);
		   }
		}
		fb_treeview_show_popup_menu (treeview, event, userdata);	
		return TRUE; /* handled */
    }
    return FALSE; /* not handled */
}

/**/
void
fb_save_list_to_file( CssedFileBrowser* fb )
{
	G_CONST_RETURN gchar* home;
	gchar* file;
	GList* open_list;
	gint i = 0;
	FILE* f;

	home = g_get_home_dir();
	file = g_strconcat(home, FB_CONFIG_FILE , NULL);
	
	f = fopen (file, "w");
	if (f != NULL)
	{
		open_list = g_list_first( fb->dirlist );
		while( open_list ){			
			fwrite ((gchar*) open_list->data, sizeof (gchar), strlen((gchar*) open_list->data), f);
			fputc ((int) '\n', f);
			open_list = g_list_next( open_list );
		}
		fclose (f);
	}
	g_free (file);
}

void
fb_load_list_from_file( CssedFileBrowser* fb )
{
	G_CONST_RETURN gchar* home;
	FILE* f;
	gchar* file, *item;
	gchar file_read[MAX_PATH];
	gint c, i = 0;

	home = g_get_home_dir();
	file = g_strconcat(home, FB_CONFIG_FILE , NULL);

	memset(file_read, 0, sizeof(gchar)*MAX_PATH);

	f = fopen (file, "r");
	if (f != NULL)
	{	
		while ( (c=fgetc(f)) != EOF )
		{
			if( c != '\n' )
			{
				file_read[i]=c;
				i++;
			}
			else
			{
				item = g_strdup(file_read);
				if( strlen(item)>0 )
					fb->dirlist = g_list_append( fb->dirlist, item );
				i=0;
				memset(file_read, 0, sizeof(gchar)*MAX_PATH);
			}
			if( i >= MAX_PATH ) break;
		}
		if((c==EOF) && (i > 0)) // no new line at end of file
		{
			item = g_strdup(file_read);
			if( strlen(file_read)>0 )
				fb->dirlist = g_list_append( fb->dirlist, item );			
		}
		fclose (f);
	}
	if( fb->dirlist != NULL )
		gtk_combo_set_popdown_strings ( GTK_COMBO( fb->combo_directory ), fb->dirlist );
	g_free (file);
}

/* 
	GsourceFunc fb_load_homedir_at_idle
	to be used by g_iddle_add
*/
gboolean
fb_load_homedir_at_idle ( gpointer fb )
{
	on_fb_button_home_clicked   (NULL, fb);
	return FALSE;
}
