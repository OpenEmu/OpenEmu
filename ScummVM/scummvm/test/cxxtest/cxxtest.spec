Name: cxxtest
Summary: CxxTest Testing Framework for C++
Version: 3.10.1
Release: 1
Copyright: LGPL
Group: Development/C++
Source: cxxtest-%{version}.tar.gz
BuildRoot: /tmp/cxxtest-build
BuildArch: noarch
Prefix: /usr

%description
CxxTest is a JUnit/CppUnit/xUnit-like framework for C++.
Its advantages over existing alternatives are that it:
 - Doesn't require RTTI
 - Doesn't require member template functions
 - Doesn't require exception handling
 - Doesn't require any external libraries (including memory management, 
   file/console I/O, graphics libraries)

%prep
%setup -n cxxtest

%build

%install
install -m 755 -d $RPM_BUILD_ROOT/usr/bin $RPM_BUILD_ROOT/usr/include/cxxtest
install -m 755 cxxtestgen.p[ly] $RPM_BUILD_ROOT/usr/bin/
install -m 644 cxxtest/* $RPM_BUILD_ROOT/usr/include/cxxtest/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(-, root, root) %doc README
%attr(-, root, root) %doc sample
%attr(-, root, root) /usr/include/cxxtest
%attr(-, root, root) /usr/bin/cxxtestgen.pl
%attr(-, root, root) /usr/bin/cxxtestgen.py
