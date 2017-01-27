%define version   1.40+
%define release   b2

Name:		kimera
Summary:	Another input method for Japanese
Version:	%{version}
Release:	%{release}
Group:		System Environment/Libraries
License:	GPL
URL:		http://kimera.sourceforge.jp/
Source0:	%{name}-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root
Requires:	qt >= 3.3.2
BuildRequires:	qt-devel >= 3.3.2
BuildArchitectures: i586

%description
Kimera is another input method for Japanese.
It supports Whiz.


%prep
%setup -q

%build
qmake "target.path=/usr/lib/kimera" "script.path=/usr/bin" "no_anthy=1" kimera.pro
sed -e "s/-O2 -g/-Os/g; s/-march=i386/-march=i586/g" -i Makefile
make clean
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

# install bin files
install -d %{buildroot}%{_bindir}
install -m 755 kimera %{buildroot}%{_bindir}

# install lib files
strip kimera-bin
install -d %{buildroot}%{_libdir}/kimera
install -m 755 kimera-bin %{buildroot}%{_libdir}/kimera/
install -d %{buildroot}%{_libdir}/kimera/dic/
install -m 644 dic/*.dic %{buildroot}%{_libdir}/kimera/dic/

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
#%doc AUTHORS COPYING README*
%{_bindir}/kimera
%{_libdir}/kimera/kimera-bin
%{_libdir}/kimera/dic/*.dic


%changelog
* Thu Nov 8 2007 Yuichiro Nakada <berry@po.yui.mine.nu>
- Added the endConvert code for WHIZ's word learning
* Wed Apr 11 2007 Yuichiro Nakada <berry@po.yui.mine.nu>
- Update to 1.40
* Sat Oct 14 2006 Yuichiro Nakada <berry@po.yui.mine.nu>
- Create for Berry Linux
