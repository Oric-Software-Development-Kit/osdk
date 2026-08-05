# Building OSDK

OSDK developers can use CMake to build the toolchain, install it, and build the sample programs with the installed tools.

It is also possible to build the OSDK with 'plain old make' and the master Makefile.

## Dependencies

Either way, you must install a C/C++ compiler, CMake, Curses, and FreeImage development files.

On Debian or Ubuntu:

```sh
sudo apt install build-essential cmake libfreeimage-dev libncurses-dev
```

On Fedora or Red Hat:

```sh
sudo dnf install gcc-c++ cmake freeimage-devel ncurses-devel
```

On macOS, install Xcode Command Line Tools and Homebrew, then:

```sh
xcode-select --install
brew install cmake freeimage
```

## Build and install

From the repository root, choose an installation directory and configure the project:

```sh
export OSDK="$PWD/.osdk"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

`OSDK` is optional. Without it, pass an explicit install prefix instead:

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/.osdk"
```

Build the complete toolchain and install it:

```sh
cmake --build build --target build_and_install
```

The `build_and_install` target builds all OSDK components before installing them into the selected prefix.

## Build all samples

After configuring the project, build every sample with the newly installed toolchain:

```sh
cmake --build build --target samples
```

The `samples` target automatically runs `build_and_install` first, discovers the installed sample CMake projects, and builds them with that installation. Sample build directories and generated `.tap`/`.dsk` files are written below `build/samples/`.

To build one installed sample manually, configure its installed directory and point it at the same OSDK prefix:

```sh
cmake -S "$OSDK/sample/c/hello_world_simple" \
  -B sample-build \
  -DOSDK_ROOT="$OSDK"
cmake --build sample-build
```

Installed samples can also infer their OSDK location from the installed `sample/cmake` helper, but setting `OSDK_ROOT` explicitly is useful when building from a copied sample tree.
