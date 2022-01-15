plugindir=$(prefix)/lib/cssed/plugins
plugin_PROGRAMS=filebrowser.so

#the core plugin files
SRCS=\
interface.c interface.h \
callbacks.c callbacks.h \
filebrowser.c filebrowser.h \
file.h dir.h \
editlist.c editlist.h

if ISDARWIN
filebrowser_so_SOURCES = $(SRCS)
filebrowser_so_LDFLAGS =  -bundle -flat_namespace -undefined suppress
filebrowser_so_LDADD = @PACKAGE_LIBS@
filebrowser_so_CFLAGS = -fno-common
else
filebrowser_so_SOURCES= $(SRCS)
filebrowser_so_LDFLAGS = -shared -fPICS
filebrowser_so_LDADD = @PACKAGE_LIBS@
endif

INCLUDES=\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/filebrowser-plugin"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@


