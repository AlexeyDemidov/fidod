Summary: fido daemon
Name: fidod
%define version 0.1
Version: %{version}
Release: 1
Copyright: GPL
Group: Network/Daemons
URL: http://home.vinf.ru/~alexd/fidod.html
Source: http://home.vinf.ru/~alexd/files/fidod-%{version}.tar.gz
# Patch:
BuildRoot: /tmp/fidod

%description
FIDO daemon

%prep
%setup

%build
./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d/
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/rc3.d/
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/rc6.d/
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/rc0.d/

make install-strip prefix=$RPM_BUILD_ROOT/usr/ ETCDIR=$RPM_BUILD_ROOT/etc/

install -o 0 -g 0 -m 755 conf/fidod.init $RPM_BUILD_ROOT/etc/rc.d/init.d/
ln -s ../init.d/fidod.init $RPM_BUILD_ROOT/etc/rc.d/rc3.d/S50fidod
ln -s ../init.d/fidod.init $RPM_BUILD_ROOT/etc/rc.d/rc0.d/K30fidod
ln -s ../init.d/fidod.init $RPM_BUILD_ROOT/etc/rc.d/rc6.d/K30fidod

%clean
rm -rf $RPM_BUILD_ROOT

%post

%postun

%files
%doc README TODO NEWS
%config /etc/fidod.conf
/usr/sbin/*
/usr/man/man8/*
/usr/info/*
/etc/rc.d/init.d/*
/etc/rc.d/rc3.d/*
/etc/rc.d/rc6.d/*
/etc/rc.d/rc0.d/*
