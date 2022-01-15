%define prefix	/usr
%define name	cssed-filebrowser-plugin
%define version	0.2.2
%define release 0%{?dist}

Summary:         Filebrowser plugin for cssed developers
Name:            %{name}
Version:         %{version}
Release:         %{release}
Group:           Development/Tools
License:         GPL
Source:          http://dl.sourceforge.net/cssed/%{name}-%{version}.tar.gz
Url:             http://cssed.sourceforge.net
BuildRoot:       %{_tmppath}/cssed-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:       i386
Requires:        gtk2 >= 2.0.6, glib2 >= 2.0, cssed >= 0.4.0
Buildrequires:   cssed-devel >= 0.4.0, gtk2-devel >= 2.0.6, glib2-devel >= 2.0


%description 
cssed is a tiny GTK+ CSS editor and validator
for web developers. This plugin provides a file 
browser panel in cssed.

%prep
%setup -q -n %{name}-%{version}

%build
[ ! -f Makefile ] || make distclean

%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} make install

%find_lang %{name}

%clean
rm -rf %{buildroot}

%files 
%defattr(-,root,root)
%{_libdir}/cssed/plugins/filebrowser.so
%doc AUTHORS COPYING ChangeLog README INSTALL NEWS

%changelog
* Tue Jul 04 2006 Iago Rubio <iago.rubio@hispalinux.es> 0.2.2-0
- Changes to use the find_lang macro. Changed to full url on "Source".
* Wed Nov 23 2005 Iago Rubio <iago.rubio@hispalinux.es> 0.2.1-0
- Version bump to fix small glitch in last release.
* Sun Sep 18 2005 Iago Rubio <iago.rubio@hispalinux.es> 0.2-0
- Updated to use cssed-0.4.0 functions.
* Wed Jun 02 2004 Iago Rubio <iago.rubio@hispalinux.es> 0.1-0
- Initiall RPM.

