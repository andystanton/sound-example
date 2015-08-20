#pragma once

#include "AudioFile.hpp"

#include "sndfile.h"
#include "al.h"
#include "alc.h"

#if defined (__linux__)
    #include <cstdlib> // required for putenv.
#endif

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using std::stringstream;
using std::string;
using std::vector;

struct Playback
{
        AudioFile * audioFile;
        int position;
        bool loop;
};

enum AudioEventType
{
        start, stop
};

class StreamHandler
{
    public:
        StreamHandler();
        ~StreamHandler();

        void processEvent(AudioEventType audioEventType,
                          AudioFile * audioFile = nullptr,
                          bool loop = false);

    private:
        ALCdevice * device;
        ALCcontext * context;
        vector<Playback *> data;
};
