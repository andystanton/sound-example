# Sound playback in C++

An example of asynchronous sound playback in C++ with Portaudio and libsndfile.

## Requirements

The plethora of build requirements are courtesy of the build process for libsndfile but are all easily obtained through your operating system's package manager.

### Source Control

- git

### Build

- c++14 compiler
- cmake >= 3.2.2
- make
- autoconf
- automake
- autogen
- libtool
- pkg-config
- python

## Build Instructions

```bash
$ git clone --recursive https://github.com/andystanton/sound-example.git
$ cd sound-example
$ ./configure
$ make
```

The example executable should now be located in `build/sound-example`.