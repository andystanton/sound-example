#include "StreamHandler.hpp"

void StreamHandler::processEvent(AudioEventType audioEventType, AudioFile * audioFile, bool loop)
{

        ALint source_state;
        ALuint buf, src;

        // switch (audioEventType) {
        // case start:
                // this->data.push_back(new Playback {
                //         audioFile,
                //         0,
                //         loop
                // });
                cout << "generating source" << endl;
                alGenSources(1, &src);
                cout << "generating buffer" << endl;
                alGenBuffers(1, &buf);

                alBufferData(buf,
                             audioFile->info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                             &audioFile->vdata.front(),
                             audioFile->vdata.size() * sizeof(uint16_t),
                             audioFile->info.samplerate);

                alSourcei(src, AL_BUFFER, buf);
                alSourcePlay(src);

                alGetSourcei(src, AL_SOURCE_STATE, &source_state);
                while (source_state == AL_PLAYING) {
                        alGetSourcei(src, AL_SOURCE_STATE, &source_state);
                }

                cout << "deleting source" << endl;
                alDeleteSources(1, &src);
                cout << "deleting buffer" << endl;
                alDeleteBuffers(1, &buf);

        //         break;
        // case stop:
        //         for (auto instance : this->data)
        //         {
        //                 delete instance;
        //         }
        //         this->data.clear();
        //         break;
        // }
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
