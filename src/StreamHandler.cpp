#include "StreamHandler.hpp"

void StreamHandler::processEvent(AudioEventType audioEventType, AudioFile * audioFile, bool loop)
{
        switch (audioEventType) {
        case start:
                data.push_back(new Playback {
                        audioFile,
                        0,
                        loop
                });

                break;
        case stop:
                for (auto instance : data)
                {
                        delete instance;
                }
                data.clear();
                break;
        }
}

StreamHandler::StreamHandler()
        : device(alcOpenDevice(alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER)))
        , context(alcCreateContext(device, nullptr))
        , data()
{
        if (!device) {
                throw string("Unable to open default device");
        }
        if (!alcMakeContextCurrent(context)) {
                throw string("Unable to set default context");
        }
}

StreamHandler::~StreamHandler()
{
        for (auto wrapper : data)
        {
                delete wrapper;
        }
        device = alcGetContextsDevice(context);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
}
