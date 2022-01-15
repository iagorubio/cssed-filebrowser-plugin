all:	dir file

dir:		
	gdk-pixbuf-csource --raw --static dir.png| sed -e "s/my_pixbuf/dir_pixbuf/g" > ../src/dir.h

file:
	gdk-pixbuf-csource --raw --static file.png| sed -e "s/my_pixbuf/file_pixbuf/g" > ../src/file.h

distdir:
	true
