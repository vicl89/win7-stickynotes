```spec
Name:           win7-stickynotes
Version:        1.0
Release:        1%{?dist}
Summary:        Plasmoide de Sticky Notes estilo Windows 7 para KDE Plasma 6

License:        GPL-3.0-or-later
URL:            https://github.com/TU_USUARIO/win7-stickynotes
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.22
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  kf6-kpackage-devel

Requires:       plasma-workspace
Requires:       qt6-qt5compat

%description
Widget de notas adhesivas (Sticky Notes) para KDE Plasma 6 inspirado en el diseño
clásico de Windows 7 Aero. Incluye soporte para formato enriquecido (negrita,
cursiva, subrayado), listas con viñetas, selector de color, tipografía y
movimiento/redimensionado libre mediante un plugin nativo en C++ y Qt 6.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
# Archivos del plasmoide (QML, config, metadata.json)
%{_datadir}/plasma/plasmoids/org.kde.plasma.win7stickynotes/

# Plugin compilado en C++ (.so, qmldir, qmltypes)
%{_libdir}/qt6/qml/org/kde/plasma/win7stickynotes/

%changelog
* Wed Oct 14 2026 Luis Fernando Vidriales Chávez - 1.0-1
- Lanzamiento inicial de Win7 Sticky Notes 1.0 para KDE Plasma 6
