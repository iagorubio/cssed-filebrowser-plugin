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

#include <gtk/gtk.h>
#include <gmodule.h>
#include <plugin.h>


#include "filebrowser.h"
#include "interface.h"
#include "callbacks.h"

G_MODULE_EXPORT CssedPlugin* init_plugin(void);
G_MODULE_EXPORT gboolean load_filebrowser ( CssedPlugin* );
G_MODULE_EXPORT void clean_filebrowser ( CssedPlugin* );

G_MODULE_EXPORT CssedPlugin filebrowser;
CssedFileBrowser* fb;

// this will return the plugin to the caller
G_MODULE_EXPORT CssedPlugin* init_plugin()
{
	
    filebrowser.name = _("Filebrowser"); 				// the plugin name	
	filebrowser.description  = _("Adds a file browser");// the plugin description
	filebrowser.load_plugin = &load_filebrowser; 	// load plugin function, will build the UI
	filebrowser.clean_plugin = &clean_filebrowser;	// clean plugin function, will destroy the UI
	filebrowser.user_data = NULL;					// User data
	filebrowser.priv =  NULL;						// Private data, this is opaque and should be ignored
	
	return &filebrowser;
}


G_MODULE_EXPORT gboolean
load_filebrowser (CssedPlugin* plugin)
{
	GtkWidget* user_interface;

	gtk_set_locale();
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

	fb = g_malloc( sizeof(CssedFileBrowser));
	fb->plugin = &filebrowser;
	fb->dirlist = NULL;
	
	user_interface = create_ui(fb);


	filebrowser.user_data = user_interface;

 
	cssed_plugin_add_page_with_widget_to_sidebar( &filebrowser, user_interface,	_("Files") );

	fb_load_list_from_file( fb );
	g_idle_add  (fb_load_homedir_at_idle,  fb);  
	                  
	return TRUE;
}

/* could be used to post UI destroy
void g_module_unload (GModule *module)
{
	g_print(_("** File browser plugin unloaded\n"));	
}
*/

// to destroy UI and stuff. called by cssed. in this example using user data
// just for fun.
G_MODULE_EXPORT void clean_filebrowser ( CssedPlugin* p )
{
	GtkWidget* ui;
	GList* list;
	GtkWidget* combo_dirs;

    ui = GTK_WIDGET( p->user_data );
	gtk_widget_destroy(	ui );

	combo_dirs = fb->combo_directory;
	
	fb_save_list_to_file( fb );
	// clean the list items
	if( fb->dirlist != NULL ){
		list = g_list_first( fb->dirlist );
		while( list ){
			g_free( list->data );
			list = g_list_next( list );
		}
		g_list_free( fb->dirlist );
	}
	return;
}
