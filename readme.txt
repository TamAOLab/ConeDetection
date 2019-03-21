This is the log of steps I took to create a development environment to
build this project.  There are possibly other combinations of versions
of libraries that might work, but these are the most recent versions
of all dependencies as of Jun 2018.

All commandline commands are made from the "x64 Native Tools Command
Prompt for VS 2017" that should be in the start menu after installing
MS Visual Studio 2017.

Give yourself at least a full day to complete these steps.

# MS Visual Studio 2017 #

- Install Microsoft Visual Studio 2017 Professional,
  selecting "Desktop development with C++".

# CMake #

- Download and Install [CMake
  3.11.0](https://cmake.org/files/v3.11/cmake-3.11.0-win64-x64.msi)

# NSIS #

- Download and Install [NSIS 3.03](http://nsis.sourceforge.net/Download)

# Qt 5 #

- Download [Qt
  5.11.1](https://download.qt.io/archive/qt/5.11/5.11.1/qt-opensource-windows-x86-5.11.1.exe)
  This needs your username and passwd. If you don't have one, set up one.

- Install.  Select at least the following (everything else can be
  unchecked, unless you want it):

  - Qt 5.11.1 -> msvc2017 64-bit 

  - ... wait hours ...

- Here are some [helpful
notes](https://wiki.qt.io/Transition_from_Qt_4.x_to_Qt5) on
transitioning a project from Qt4 to 5.

# Vtk #
   *NOTE* VTK is built and linked statically for MacOS, dynamically for Windows.

There are some slightly out-of-date notes about [VTK
installation](http://www.vtk.org/Wiki/VTK/Configure_and_Build).

- Download [Vtk 8.1.1](http://www.vtk.org/files/release/8.1/VTK-8.1.1.zip).

- Unzip it.

- Go to Cmake installation folder, run `cmake-gui`

  - In the source directory field: Where you unzipped VTK. Example: C:/VTK/VTK-8.1.1

  - In the build directory field: Where you want to put CMake files for the VTK build, and 
    select it / Create it if not exist. Example: C:/VTK/VTKBuild/CMakeFiles

  - Click "Configure"

  - Select "Visual Studio 15 2017 `Win64`" as generator for this project

  - ... wait hours ...

  - Add the following configuration options by clicking "Add Entry"
    for each of the following:

    - `VTK_QT_VERSION`, STRING, 5
    - `QT_QMAKE_EXECUTABLE`, Filepath, set to qmake.exe in your QT5 MSVC 2017 directory.
      For example, `C:\Qt\5.11.1\msvc2017_64\bin\qmake.exe`
    - `CMAKE_PREFIX_PATH`: PATH, `C:\Qt\5.11.1\msvc2017_64\lib\cmake\Qt5` for example.

    - `Qt5_DIR`, PATH,
      Your QT5 installation folder +`\5.11.1\msvc2017_64\lib\cmake\Qt5\` (This is optional
      if `CMAKE_PREFIX_PATH` is set)

  - Make sure the `VTK_Group_Qt` checkbox is checked.

  - Change the value of `VTK_RENDERING_BACKEND` to "OpenGL2" (not "OpenGL 2") AFTER VTK 8.1

  - Check "Advanced", check the box for `VTK_LEGACY_SILENT`
    (this is to suppress VTK warnings about deprecations)

  - (optionally) change CMAKE_INSTALL_PREFIX to where you want to install VTK (instead of C:/Program Files/VTK`).
    Example: `C:/VTK/VTKBuild`

  - Click "Configure" again

  - Click "Generate"

  - Close CMake

- Start Visual Studio 2017 with administrator privileges
    (Right-click on Visual Studio in the Start Menu, choose "Run as
    administrator")

  - Open 'VTK.sln' in your VTK build folder
  
  - Check the platform in MSVC to make sure it's what you want. For example, make sure it's `Win64`

  - Choose `Release` mode, right-click "ALL_BUILD" and choose "Build"

  - ... wait hours ...
  
  - Right-click on "INSTALL", choose "Build" if you can not find it's installed.
  
  - Choose 'Debug' mode, make a build.
  
  - ... wait for some time until finish ...

  - Add the VTK installation folder + `/bin/Release` to PATH in your system environment 

# Itk #

- Download [Itk
  4.13.0](https://sourceforge.net/projects/itk/files/itk/4.13/InsightToolkit-4.13.0.zip/download)

- Unzip it.

- Run `cmake-gui` in your CMake installation directory

  - In the source directory field: Where you unzipped ITK. Example: C:/ITK/InsightToolkit-4.13.1

  - In the build directory field: Where you want to install CMake files for the ITK build,
    and select it / Create it if not exist. Example: C:/ITK/ITKBuild/CMakeFiles

  - Click "Configure"

  - Select "Visual Studio 15 2017 Win64" as generator for this project

  - ... wait hours ...

  - Select `Module_ITKV3Compatibility`, and `ITKV3_COMPATIBILITY`. If version is greater than 5, it may
    say this is outdated.

  - Check "Advanced", and make sure `Module_ITKVtkGlue` is checked

  - Click "Configure"

  - Click "Generate"

  - Close CMake

- Start Visual Studio 2017 with administrator privileges (Right-click on Visual Studio in the Start Menu, choose "Run as
    administrator")

  - Open `ITK.sln` inside the ITK build directory

  - Make sure `Release` is selected and platform is `Win64`.  Then, on the right hand side,
    right-click "ALL_BUILD" and choose "Build"

  - ... wait hours ...

  - Right-click on "INSTALL", choose "Build" if you can not find it's installed.
  
  - Choose `Debug` mode, right-click "ALL_BUILD" and choose "Build"
	
  - Quit Visual Studio

# ConeDetection #

- Run `cmake-gui` in your CMake installation directory

  - For source directory, select the directory where `CMakelists.txt` file is within the project.

  - For build directory, create a new sub-directory called `build` under the source directory.

  - Add the value: `CMAKE_PREFIX_PATH`, PATH,
    Your QT5 installation folder + `\msvc2017_64\lib\cmake\Qt5\` (This assumes
    you installed Qt to the default location)

  - Add the value: `VTK_DIR`, PATH, {Your VTK build folder containing CMakeCache.txt}
    (e.g. `C:\Softs\VTK` or `C:\VTK\VTKBuild\CMakeFiles`)

  - Add the value: `ITK_DIR`, PATH, {Your ITK build folder containing CMakeCache.txt}
    (e.g. `C:\Softs\ITK` or `C:\ITK\ITKBuild\CMakeFiles`)

  - Make sure your compiler is VS 2017 Win64.
  
  - Click "Configure"

  - Click "Generate"
  
  - Close it
  
- Start Visual Studio 2017 with administrator privileges (Right-click on Visual Studio in the Start Menu, choose "Run as
    administrator")

  - Open `ConeDetection.sln` in build directory and make sure the platform is Win64.

  - Choose `Release` mode, right-click "ALL_BUILD" and choose "Build"
  
  - Right-click on "PACKAGE" and select "Build".
This will build the distribution package "ConeDetection-<X.X.X>-win64.exe" in the build directory.
  
  - Choose `Debug` mode, right-click "ALL_BUILD" and choose "Build"
  
  - If start local debug, it will take much longer time comparing with executing release.

# Note #
- For the future version, although the versions of tools/packages/components 
  are different, the above steps are similar.


### Building for MacOS ###
   *NOTE* VTK is built and linked statically

1. Install Xcode
2. Install CMake
3. Download and install Qt 5 (Checking "macOS" under the latest 5.x version is sufficient)
4. Download and build VTK 8 (both Debug and Release) with the following CMake options checked:
   `VTK_Group_Qt`
   `VTK_LEGACY_SILENT` (Advanced)
and the `BUILD_SHARED_LIBS` option *unchecked* (unlike in the Windows build!)
5. Download and build ITK 4 (both Debug and Release) with the following CMake options checked:
   `Module_ITKV3Compatibility`
   `ITKV3_COMPATIBILITY`
   `Module_ITKVtkGlue` (Advanced)
6. Build cone-detection project by running CMake, selecting Xcode as compiler,
and providing the following options:
   `Qt5_DIR`:PATH - Qt cmake dir (containing Qt5Config.cmake), e.g. "/Users/Shared/Qt/5.12.0/clang_64/lib/cmake/Qt5/"
   `VTK_DIR`:PATH - VTK build dir (containing CMakeCache.txt), e.g. "/Users/Shared/VTK/build/"
   `ITK_DIR`:PATH - ITK build dir (containing CMakeCache.txt), e.g. "/Users/Shared/ITK/build/"
7. To make the distribution package with Xcode, switch ALL_BUILD, install and PACKAGE targets to Release mode,
then build target PACKAGE. The result is "ConeDetection-<X.X.X>-Darwin.dmg" in the build directory.
If Xcode asks for permission to access "Finder", answer "Yes". 

