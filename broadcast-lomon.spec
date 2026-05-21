Name:           broadcast-lomon
Version:        1.0
Release:        1%{?dist}
Summary:        Secure terminal broadcast chat system

License:        GPL
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  make
# Якщо юзаєш якісь системні ліби типу openssl, їх треба тут вказати. 
# Якщо FTXUI тягнеться самим CMake (FetchContent), то додаткових ліб не треба.

%description
Terminal-based broadcast chat system with RSA/AES encryption and dynamic ban mechanics.

%prep
# Ця команда розпаковує архів Source0
%autosetup

%build
# Створюємо папку build і компілимо твоїм CMake
mkdir build && cd build
cmake ..
make %{?_smp_mflags}

%install
# Закидаємо готовий бінарник в /usr/bin, щоб він запускався командою з будь-якого місця
mkdir -p %{buildroot}/usr/bin
install -m 0755 build/broadcast %{buildroot}/usr/bin/broadcast-lomon

%files
# Вказуємо, які файли належать пакету
%{_bindir}/broadcast-lomon