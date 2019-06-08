#include "StreamHandler.hpp"

#include <sstream>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <util.hpp>

int StreamHandler::PortAudioCallback(
    const void * input,
    void * output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo * paTimeInfo,
    PaStreamCallbackFlags statusFlags,
    void * userData
)
{
    auto * handler = (StreamHandler *) userData;

    unsigned long stereoFrameCount = frameCount * handler->CHANNEL_COUNT;
    std::memset((int *) output, 0, stereoFrameCount * sizeof(int));

    if (!handler->playingSounds.empty()) {
        auto it = handler->playingSounds.begin();
        while (it != handler->playingSounds.end()) {
            Playback & data = *it;
            AudioFile * audioFile = data.audioFile;

            float outputBuffer[stereoFrameCount];
            float * bufferCursor = outputBuffer;

            unsigned long framesLeft = frameCount;
            unsigned long framesRead;

            bool playbackEnded = false;
            while (framesLeft > 0) {
                sf_seek(audioFile->data, data.position, SEEK_SET);

                if (framesLeft > (audioFile->info.frames - data.position)) {
                    framesRead = (unsigned int) (audioFile->info.frames - data.position);
                    if (data.loop) {
                        data.position = 0;
                    } else {
                        playbackEnded = true;
                        framesLeft = framesRead;
                    }
                } else {
                    framesRead = framesLeft;
                    data.position += framesRead;
                }

                sf_readf_float(audioFile->data, bufferCursor, framesRead);

                bufferCursor += framesRead;

                framesLeft -= framesRead;
            }

            auto * outputCursor = (float *) output;
            if (audioFile->info.channels == 1) {
                for (unsigned long i = 0; i < stereoFrameCount; ++i) {
                    *outputCursor += (0.5 * outputBuffer[i]);
                    ++outputCursor;
                    *outputCursor += (0.5 * outputBuffer[i]);
                    ++outputCursor;
                }
            } else {
                for (unsigned long i = 0; i < stereoFrameCount; ++i) {
                    *outputCursor += (0.5 * outputBuffer[i]);
                    ++outputCursor;
                }
            }

            if (playbackEnded) {
                it = handler->playingSounds.erase(it);
            } else {
                ++it;
            }
        }
    }
    return paContinue;
}

void StreamHandler::processEvent(AudioEventType audioEventType, AudioFile * audioFile, bool loop)
{
    switch (audioEventType) {
        case start:
            if (Pa_IsStreamStopped(stream)) {
                playingSounds.clear();
                util::wrapPortAudioCall("start stream", [&]() { return Pa_StartStream(stream); });
            }
            if (playingSounds.size() <= 2) {
                playingSounds.push_back(
                    Playback {
                        .audioFile = audioFile,
                        .position = 0,
                        .loop = loop,
                    }
                );
            }
            break;
        case stop:
            if (!Pa_IsStreamStopped(stream)) {
                util::wrapPortAudioCall("stop stream", [&]() { return Pa_StopStream(stream); });
                playingSounds.clear();
            }
            break;
    }
}

StreamHandler::StreamHandler()
    : playingSounds()
{

#if defined (__linux__)
    putenv((char *) "PULSE_LATENCY_MSEC=10");
#endif

    util::wrapPortAudioCallOrTerminate("initialize", []() { return Pa_Initialize(); });

    PaStreamParameters outputParameters {
        .device = Pa_GetDefaultOutputDevice(),
        .channelCount = CHANNEL_COUNT,
        .sampleFormat = paFloat32,
        .suggestedLatency = 0.01,
        .hostApiSpecificStreamInfo = nullptr,
    };

    util::wrapPortAudioCallOrTerminate("open", [&]() {
        return Pa_OpenStream(
            &stream,
            NO_INPUT,
            &outputParameters,
            SAMPLE_RATE,
            paFramesPerBufferUnspecified,
            paNoFlag,
            &PortAudioCallback,
            this
        );
    });
}

StreamHandler::~StreamHandler()
{
    processEvent(AudioEventType::stop);

    util::wrapPortAudioCallOrTerminate("close", [&]() { return Pa_CloseStream(stream); });
    util::wrapPortAudioCall("terminate", []() { return Pa_Terminate(); });
    stream = nullptr;
}
