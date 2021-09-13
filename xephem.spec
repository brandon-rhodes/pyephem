Name:		xephem
Version:	4.1.0
Release:        1
Summary:        XEphem is an interactive astronomy program for all UNIX platforms.
License:	MIT

URL:            https://github.com/XEphem/XEphem

Source0:        %{name}-%{version}.tgz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:	motif-devel

Requires:	motif

%description
XEphem is an interactive astronomy program for all UNIX platforms. Originally
written by Elwood Downey in the 1990s, it has graciously been released under the
MIT License. It is now maintained by an "XEphem" organization, with its source
code residing at https://github.com/XEphem/XEphem.

%if 0%{?rhel} >= 7
%global motifl %{_libdir}
%endif

%prep
%setup -q

%build
cd GUI/xephem
make MOTIFL=%{motifl}
gzip -c %{name}.man > %{name}.1x.gz

%install
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_mandir}/man1x/

cd GUI/xephem
install -m 0755 %{name} %{buildroot}/%{_bindir}
install -m 0444 %{name}.1x.gz %{buildroot}/%{_mandir}/man1x/

%files
%{_bindir}/%{name}
%{_mandir}/man1x/%{name}.1x.gz

%changelog
* Fri Mar 05 2021 Douglas Needham <cinnion+github@gmail.com> 4.0.1-1
- New RPM package built with tito


