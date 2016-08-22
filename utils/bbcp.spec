Name:        bbcp2
Version:     1.0
Release:     2
Vendor:      Pal Mezei
License:     GNU LGPL v3
Group:       External packages
URL:         https://github.com/sirpalee/bbcp2
Packager:    Rares Hornet
Source:      %{name}-%{version}.tar.gz
BuildRoot:   %{_tmppath}/%{pkg}-%{version}
Prefix:      /usr
Summary:     Securely and quickly copy data from source to target.

%description
bbcp2 is a point-to-point network file copy application written by Andy Hanushevsky at SLAC and improved by Pal Mezei
It is capable of transferring files at approaching line speeds in the WAN.

%define instdir  /usr

%prep
%setup -n %{name}-%{version} -q

%build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{buildroot}
make

%install
install -d %{buildroot}/%{instdir}/bin
cp bbcp2 %{buildroot}/%{instdir}/bin

%clean
rm -rf %{buildroot}

%files
%{instdir}/bin/*

%changelog

* Wed Aug 13 2016 Pal Mezei
- cleaned up code and added user ownership (as root/sudo only) feature into preserve flag
