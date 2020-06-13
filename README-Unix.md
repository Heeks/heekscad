# HeeksCAD

This file describes how to build and install HeeksCAD under Unix systems.

## Requirements

To build HeeksCAD, you need to install these requirements (with develoment files):

  * OpenCASCADE or OCE (OpenCASCADE Community Edition)
  * wxWidgets 2.8 or 3.0
  * [libarea](https://github.com/Heeks/heekscad.wiki.git)

## Preparation

Create a build directory (e.g. `build/` in sources root directory):

```shell
mkdir build
cd build
```

## Configure build

If you want a default prefix (/usr/local) and a "Release" type, simply run:

```shell
cmake ..
```

If you want to change install prefix (e.g. /usr):

```shell
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```

If you want to debug HeeksCAD and its install:

```shell
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PWD/install ..
```

## Build

After a successful CMake configuration, you can build it using:

```shell
make
``

If you want more output (ie. to debug):

```shell
make VERBOSE=1
```

## Install

Using default or system-wide prefix:

```shell
sudo make install
```

Please note that if you installed it in `/usr/local`, you may need to run:

```shell
sudo ldconfig
```

If you choose a user-writable prefix, superuser privileges are not needed:

```shell
make install
```

## One-liner snippets

### Default

```shell
mkdir build && cd build && cmake .. && make
```

### Debug

```shell
mkdir debug && cd debug && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PWD/install .. && make && make install
LD_LIBRARY_PATH=install/lib install/bin/heekscad
```

## Translation

Merge new strings:
 * Generate a .pot file and merge it to your catalog

```shell
sh translations/generate-pot-file.sh translations/xx/HeeksCAD.po
```

Note: this script assumes you have `heekscad` and `heekscnc` in the same parent directory.
 
 * After translating them, you can test catalog compilation using:

```shell
msgfmt -o HeeksCAD.mo HeeksCAD.po
```

Note: if you launch `heekscad` from a folder that contains `HeeksCAD.mo` file, this catalog will be loaded instead of system-wide one.
