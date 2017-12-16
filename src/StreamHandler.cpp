#include "StreamHandler.hpp"

#include <sstream>
#include <cstring>
#include <stdexcept>

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

    if (!handler->data.empty()) {
        auto it = handler->data.begin();
        while (it != handler->data.end()) {
            Playback & data = *it;
            AudioFile * audioFile = data.audioFile;

            float outputBuffer[stereoFrameCount];
            float * bufferCursor = outputBuffer;

            auto framesLeft = (unsigned int) frameCount;
            unsigned int framesRead;

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
                it = handler->data.erase(it);
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
                PaError startError = Pa_StartStream(stream);
                if (startError) {
                    std::stringstream errorMessage;
                    errorMessage << "Unable to start PortAudio stream (" << startError << ": " << Pa_GetErrorText(startError) << ")";
                    throw std::runtime_error(errorMessage.str());
                }
            }
            data.push_back(
                    Playback {
                            audioFile,
                            0,
                            loop
                    }
            );
            break;
        case stop:
            if (!Pa_IsStreamStopped(stream)) {
                PaError stopError = Pa_StopStream(stream);
                if (stopError) {
                    std::stringstream errorMessage;
                    errorMessage << "Unable to stop PortAudio stream (" << stopError << ": " << Pa_GetErrorText(stopError) << ")";
                    throw std::runtime_error(errorMessage.str());
                }
                data.clear();
            }
            break;
    }
}

StreamHandler::StreamHandler()
        : data()
{

#if defined (__linux__)
    putenv("PULSE_LATENCY_MSEC=10");
#endif

    PaError startError = Pa_Initialize();
    if (startError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to initialise PortAudio (" << startError << ": " << Pa_GetErrorText(startError) << ")";
        PaError terminateError = Pa_Terminate();
        if (terminateError) {
            errorMessage << "\n" << "Unable to terminate PortAudio (" << terminateError << ": " << Pa_GetErrorText(terminateError) << ")";
        }
        throw std::runtime_error(errorMessage.str());
    }

    PaStreamParameters outputParameters {};

    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = CHANNEL_COUNT;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = 0.01;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaError openStreamError = Pa_OpenStream(
            &stream,
            NO_INPUT,
            &outputParameters,
            SAMPLE_RATE,
            paFramesPerBufferUnspecified,
            paNoFlag,
            &PortAudioCallback,
            this
    );

    if (openStreamError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to open PortAudio stream for output (" << openStreamError << ": " << Pa_GetErrorText(openStreamError) << ")";
        PaError terminateError = Pa_Terminate();
        if (terminateError) {
            errorMessage << "\n" << "Unable to terminate PortAudio (" << terminateError << ": " << Pa_GetErrorText(terminateError) << ")";
        }
        throw std::runtime_error(errorMessage.str());
    }
}

StreamHandler::~StreamHandler()
{
//    processEvent(AudioEventType::stop);

    PaError closeStreamError = Pa_CloseStream(stream);
    if (closeStreamError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to close PortAudio stream (" << closeStreamError << ": " << Pa_GetErrorText(closeStreamError) << ")";
        PaError terminateError = Pa_Terminate();
        if (terminateError) {
            errorMessage << "\n" << "Unable to terminate PortAudio (" << terminateError << ": " << Pa_GetErrorText(terminateError) << ")";
        }
        throw std::runtime_error(errorMessage.str());
    }

    PaError terminateError = Pa_Terminate();
    if (terminateError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to terminate PortAudio (" << terminateError << ": " << Pa_GetErrorText(terminateError) << ")";
        throw std::runtime_error(errorMessage.str());
    }
}
