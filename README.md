# Sound playback in C++

An example of asynchronous sound playback in C++ with Portaudio and libsndfile.

## Build

### Requirements

- git
- C++14 compiler (or newer)
- CMake >= 3.15
- Build system (make, ninja, or IDE)

**Note:** Dependencies (libsndfile and PortAudio) are automatically fetched and built by CMake - no manual installation required.

### Build Process

```bash
./configure
make
```

## Run

```bash
cd build
./sound-example
```

This starts the demo terminal app with the following options:

```
Playback with portaudio and libsndfile

Options:
 O: stereo sound
 P: mono sound
 L: loop sound
 K: stop all sounds

Press 'Q' to quit
```
