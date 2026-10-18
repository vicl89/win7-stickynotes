Name:           win7-stickynotes
Version:        1.1
Release:        1%{?dist}
Summary:        Plasmoide de Sticky Notes estilo Windows 7 para KDE Plasma 6

License:        GPL-3.0-or-later
URL:            https://github.com/vicl89/win7-stickynotes
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
cursiva, subrayado), listas con viñetas, listas de tareas interactivas (checklist)
con tachado automático, selector de color, tipografía y persistencia de posición.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
%{_datadir}/plasma/plasmoids/org.kde.plasma.win7stickynotes/
%{_libdir}/qt6/qml/org/kde/plasma/win7stickynotes/

%changelog
* Sat Oct 17 2026 Luis Fernando Vidriales Chávez - 1.1-1
- Añadido soporte para listas de tareas (checklist) con tachado automático al hacer clic
- Corregida la aplicación de formato al seleccionar texto arrastrando el ratón
- Corregida la persistencia de posición de las notas al reiniciar sesión

* Wed Oct 14 2026 Luis Fernando Vidriales Chávez - 1.0-1
- Lanzamiento inicial de Win7 Sticky Notes 1.0 para KDE Plasma 6
