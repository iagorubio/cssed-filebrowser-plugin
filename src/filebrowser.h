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

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

typedef struct _CssedFileBrowser {
  CssedPlugin* plugin;
  GtkWidget* entry_directory;
  GtkWidget* combo_directory;
  GtkWidget* checkbox_hidden;
  GtkWidget* entry_pattern;
  GtkWidget* button_clear_pattern;
  GtkListStore* dir_store;
  GtkListStore* file_store;
  GtkWidget *treeview_files;
  GtkWidget *treeview_directories;
  GdkPixbuf* dir_img;
  GdkPixbuf* file_img;  
  gchar* current_dir;
  GList* dirlist;
} CssedFileBrowser;

#ifndef WIN32
typedef struct _FilePermsUI {
  GtkWidget *checkbutton_owner_read;
  GtkWidget *checkbutton_group_read;
  GtkWidget *checkbutton_others_read;
  GtkWidget *checkbutton_owner_write;
  GtkWidget *checkbutton_group_write;
  GtkWidget *checkbutton_others_write;
  GtkWidget *checkbutton_owner_execute;
  GtkWidget *checkbutton_group_execute;
  GtkWidget *checkbutton_others_execute;
  GtkWidget *checkbutton_setuid;
  GtkWidget *checkbutton_setgid;
  GtkWidget *checkbutton_sticky_bit;
} FilePermsUI;
#endif
