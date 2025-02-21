# C++ utilities
Useful C++ classes and routines such as argument parser, IO and conversion utilities.

## Features
The library contains helpers for:

* parsing command-line arguments and providing Bash completion
    - supports nested arguments
    - supports operations (no `--` or `-` prefix, eg. `git status`)
    - can check for invalid or uncombinable arguments
    - can print help automatically
    - provides automatic Bash completion for argument names
    - allows customizing Bash completion for argument values
* dealing with dates and times
* conversion of primitive data types to byte-buffers and vice versa (litte-endian and big-endian)
* common string conversions/operations, eg.
    - character set conversions via iconv
    - split, join, find and replace
    - conversion from number to string and vice verca
    - encoding/decoding base-64
    - building string without multiple heap allocations ("string builder")
* using standard IO streams
    - reading/writing primitive data types of various sizes (little-endian and big-endian)
    - reading/writing terminated strings and size-prefixed strings
    - reading/writing INI files
    - reading bitwise (from a buffer; not using standard IO streams)
    - writing formatted output using ANSI escape sequences
    - instantiating a standard IO stream from a native file descriptor to support UTF-8 encoded
      file paths under Windows and Android's `content://` URLs
* using SFINAE by providing additional traits, eg. for checking whether a type is iterable
* testing with *CppUnit*
    - finding testfiles and make working copies of testfiles
    - assert standard output
    - various helper
* building with CMake by providing some modules and templates

Besides, the library provides a few useful algorithms and data structures:

* min(), max() for any number of arguments
* digitsum(), factorial(), powerModulo(), inverseModulo(), orderModulo()
* Damerau–Levenshtein distance
* *N*-dimensional array

## API/ABI stability
The following counts for `c++utilities` and my other libraries unless stated otherwise:

* Different major versions are incompatible (API- and ABI-wise). Different major versions can be
  installed within the same prefix using the CMake variable `CONFIGURATION_NAME` (see documentation
  about build variables mentioned below).
* Minor versions are backwards compatible (API- and ABI-wise) to previous ones within the same major
  version.
* Patch versions are interchangeable (API- and ABI-wise) within the same major/minor version.
* Some functions or classes are experimental. They might be modified in an incompatible way or even
  removed in the next minor or patch release.

## Build instructions
These build instructions apply to `c++utilities` but also to my other projects using it.

### Requirements
#### Build-only dependencies
* C++ compiler supporting C++17, tested with
    - g++ to compile for GNU/Linux and Windows
    - clang++ to compile for GNU/Linux and Android
* CMake (at least 3.3.0) and Ninja or GNU Make
* cppunit for unit tests (optional)
* Doxygen for API documentation (optional)
* Graphviz for diagrams in the API documentation (optional)
* clang-format for tidying (optional)
* llvm-profdata, llvm-cov and cppunit for source-based code coverage analysis (optional)
* [appstreamcli](https://www.freedesktop.org/wiki/Distributions/AppStream/) for validation
  of generated AppStream files (optional)

#### Runtime dependencies
* The `c++utilities` library itself only needs
    * C++ standard library supporting C++17, tested with
        - libstdc++ under GNU/Linux and Windows
        - libc++ under GNU/Linux and Android
    * glibc with iconv support or standalone iconv library
    * libstdc++ or Boost.Iostreams for `NativeFileStream` (optional)
* My other projects have further dependencies such as Qt. Checkout the README of these
  projects for further details.

### How to build
Example using Ninja:
```
cmake -G Ninja \
      -S "path/to/source/directory" \
      -B "path/to/build/directory" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="/final/install/location"
make # build the binaries
cmake --build "path/to/build/directory"
# format source files (optional, must be enabled via CLANG_FORMAT_ENABLED)
cmake --build "path/to/build/directory" --target tidy
# build and run tests (optional)
cmake --build "path/to/build/directory" --target check
# build and run tests measuring test coverage (optional, must be enabled via CLANG_SOURCE_BASED_COVERAGE_ENABLED)
cmake --build "path/to/build/directory" --target coverage
# build API documentation (optional)
cmake --build "path/to/build/directory" --target apidoc
# install binaries, headers and additional files
DESTDIR="/temporary/install/location" \
  cmake --install "path/to/build/directory"
```

#### General notes
* ```LIB_SUFFIX```, ```LIB_SUFFIX_32``` and ```LIB_SUFFIX_64``` can be set to specify a suffix
  for the library directory, eg. lib*64* or lib*32*. The 32/64 variants are only used when building
  for 32/64-bit architecture.
* By default the build system will *build* static libs. To *build* shared libraries *instead*, set
  `BUILD_SHARED_LIBS=ON`.
* By default the build system will prefer *linking against* shared libraries. To force *linking against*
  static libraries set `STATIC_LINKAGE=ON`. However, this will only affect applications. To force linking
  statically when building shared libraries set `STATIC_LIBRARY_LINKAGE=ON`.
* If thread local storage is not supported by your compiler/platform (might be the case on MacOS), you can
  disable making use of it via `ENABLE_THREAD_LOCAL=OFF`.
* To disable use of `std::filesystem`, set `USE_STANDARD_FILESYSTEM=OFF`. Note that the Bash completion will
  not be able to suggest files and directories with `USE_STANDARD_FILESYSTEM=OFF`.
* To disable `NativeFileStream` (and make it just a regular `std::fstream`), set `USE_NATIVE_FILE_BUFFER=OFF`.
  Note that handling paths with non-ASCII characters will then cease to work on Windows.
* The Qt-based applications support bundeling icon themes by specifying e.g.
  `BUILTIN_ICON_THEMES=breeze;breeze-dark`.
    * This variable must be set when building the application (not when building any of the libraries).
    * The specified icon themes need to be installed in the usual location. Otherwise, use e.g.
      `DBUILTIN_ICON_THEMES_SEARCH_PATH=D:/programming/misc/breeze-icons/usr/share/icons` to specify the
      search path.
* For more detailed documentation, see the documentation about build variables (in
  [directory doc](https://github.com/Martchus/cpp-utilities/blob/master/doc/buildvariables.md) and
  in Doxygen version accessible via "Related Pages").
* The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains build scripts for GNU/Linux,
  Android, Windows and MacOS X in form of Arch Linux packages using `ninja`. These scripts can be used as an
  example also when building under/for other platforms.

#### Windows-specific notes
* To create application icons the tool `ffmpeg`/`avconv` is required.
* Windows builds are mainly conducted using mingw-w64/GCC so using them is recommended. Building with MSVC
  should be possible as well but it is not as well tested.
* When using `BUILTIN_ICON_THEMES`, the icon theme still needs to be installed as if it was installed on a
  GNU/Linux system. So simply grab e.g. the Arch Linux package `breeze-icons` and extract it somewhere. Do
  *not* use the package from MSYS2 or what comes with builds from KDE's binary factory.

#### MacOS-specific notes
* To create application icons the tool `png2icns` is required.
* Building for MacOS X under GNU/Linux is possible using [osxcross](https://github.com/tpoechtrager/osxcross).
* MacOS X builds are not tested regularly but should generally work (maybe with minor tweaks necassary).
* There is a [Homebrew formula](https://gist.github.com/rakkesh/0b13b8fca5dd1d57d98537ef1dd2e0dd) to
  build Tag Editor (without GUI).
* There are [MacPorts packages](https://www.macports.org/ports.php?by=name&substr=syncthingtray-devel)
  to build Syncthing Tray.

#### Development builds
During development I find it useful to build all required projects (for instance c++utilities, qtutilities,
tagparser and tageditor) as one big project.

This can be easily achieved by using CMake's `add_subdirectory()` function. For project files see the repository
[subdirs](https://github.com/Martchus/subdirs). For an example, see
[build instructions for Syncthing Tray](https://github.com/Martchus/syncthingtray#building-this-straight) or
[build instructions for Tag Editor](https://github.com/Martchus/tageditor#building-this-straight).

For a debug build, use `-DCMAKE_BUILD_TYPE=Debug`. To tweak various settings (e.g. warnings) for development,
use `-DENABLE_DEVEL_DEFAULTS=ON`.

#### CMake presets
There are some generic [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) available
but also some specific to certain Arch Linux packaging found in the AUR and my PKGBUILDs repository.

Use `cmake --list-presets` to list all presets. All `cmake` commands need to be executed within the source
directory. Builds will be created within a sub-directory of the path specified via the environment variable
`BUILD_DIR`. Here is an example for creating a build with the `arch-static-compat-devel` preset and invoking
tests:

```
export BUILD_DIR=$HOME/builds                                   # set build directory via environment variable
cmake --preset arch-static-compat-devel                         # configure build
cmake --build --preset arch-static-compat-devel -- -v           # conduct build
cmake --build --preset arch-static-compat-devel --target check  # run tests
cmake --build --preset arch-static-compat-devel --target tidy   # apply formatting
```

This preset is quite special (see [PKGBUILDs](https://github.com/Martchus/PKGBUILDs#static-gnulinux-libraries)
for details about it). The most useful presets for development are likely `devel`, `devel-qt6` and `debug`.

Note that these presets are supposed to cover all of my projects (so some of them aren't really making a
difference when just building `c++utilities` itself). To use presets in other projects, simply symlink the
file `CMakePresets.json` into the source directory of those projects which works with the "subdirs" projects
mentioned in the previous section as well.

Note that the `devel` preset (and all presets inheriting from it) use `ccache` which therefore needs to be
installed.

The `win-x64-msvc-static` preset (and all presets inheriting from it) need various additional environment
variables to be set:
* `MSYS2_ROOT`: for Perl (only used by `qtforkawesome` so far), `clang-format`, Doxygen and FFmpeg provided
  via MSYS2 packages; install the following packages:
  ```
  pacman -Syu perl mingw-w64-x86_64-clang-tools-extra mingw-w64-x86_64-doxygen mingw-w64-x86_64-ffmpeg
  ```
* `MSVC_ROOT`: for compiler and stdlib usually installed as part of Visual Studio setup, e.g.
  `C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.34.31933`
* `WIN_KITS_ROOT`: for Windows platform headers/libraries usually installed as part of Visual Studio setup,
  e.g. `C:/Program Files (x86)/Windows Kits/10`
* `QT_ROOT`: for Qt libraries provided by official Qt installer, e.g. `D:/programming/qt/6.5.0/msvc2019_64`
* `QT_TOOLS`:  for additional build tools provided by official Qt installer, e.g. `D:/programming/qt/Tools`
* `VCPKG_ROOT`: directory of VCPKG checkout used for other dependencies; install the following packages:
  ```
  vcpkg install boost-system:x64-windows-static boost-iostreams:x64-windows-static boost-filesystem:x64-windows-static boost-hana:x64-windows-static boost-process:x64-windows-static boost-asio:x64-windows-static libiconv:x64-windows-static zlib:x64-windows-static openssl:x64-windows-static cppunit:x64-windows-static
  ```

### Packaging
The mentioned repositories contain packages for `c++utilities` itself but also for my other projects.
However, the README files of my other projects contain a more exhaustive list.

#### Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files for building Arch Linux
packages of the latest release and the Git master.

PKGBUILDs to cross compile for Android, Windows (using mingw-w64) and for MacOS X (using osxcross) are
included as well.

#### RPM packages for openSUSE and Fedora
RPM \*.spec files can be found at [openSUSE Build Servide](https://build.opensuse.org/project/show/home:mkittler).
Packages are available for several architectures.

There is also a [sub project](https://build.opensuse.org/project/show/home:mkittler:vcs) containing the builds
from the Git master branch.

#### Gentoo
Checkout [Case_Of's overlay](https://codeberg.org/Case_Of/gentoo-overlay)
or [perfect7gentleman's overlay](https://gitlab.com/Perfect_Gentleman/PG_Overlay).

## Copyright notice and license
Copyright © 2015-2023 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
