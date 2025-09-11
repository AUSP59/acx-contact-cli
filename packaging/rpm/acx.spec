Name:           acx
Version:        1.0.0
Release:        1%{?dist}
Summary:        ACX Enterprise Contact CLI

License:        ASL 2.0
URL:            https://example.com/acx
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake, gcc-c++, ninja-build

%description
Auditable, portable contact CLI with elite OSS governance & security.

%prep
%setup -q

%build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

%install
mkdir -p %{buildroot}/%{_bindir}
install -m 0755 build/acx %{buildroot}/%{_bindir}/acx

%files
%{_bindir}/acx

%changelog
* {today} ACX Team <maintainers@example.com> - 1.0.0-1
- Initial package
