#include "StreamHandler.hpp"
#include <stdexcept>
#include <sstream>

using std::stringstream;

int StreamHandler::PortAudioCallback(
    const void * input,
    void * output,
    const unsigned long frameCount,
    const PaStreamCallbackTimeInfo * paTimeInfo,
    PaStreamCallbackFlags statusFlags,
    void * userData
) {
    auto * handler = static_cast<StreamHandler *>(userData);

    const unsigned long stereoFrameCount = frameCount * handler->CHANNEL_COUNT;
    std::memset(output, 0, stereoFrameCount * sizeof(int));

    std::lock_guard<std::mutex> lock(handler->dataMutex);

    if (!handler->data.empty()) {
        auto it = handler->data.begin();
        while (it != handler->data.end()) {
            Playback * data = it->get();
            const AudioFile * audioFile = data->audioFile;

            std::vector<int> outputBuffer(stereoFrameCount);
            int * bufferCursor = outputBuffer.data();

            auto framesLeft = static_cast<unsigned int>(frameCount);
            unsigned int framesRead;

            bool playbackEnded = false;
            while (framesLeft > 0) {
                sf_seek(audioFile->getData(), data->position, SEEK_SET);

                if (framesLeft > (audioFile->getInfo().frames - data->position)) {
                    framesRead = static_cast<unsigned int>(
                        audioFile->getInfo().frames - data->position);
                    if (data->loop) {
                        data->position = 0;
                    } else {
                        playbackEnded = true;
                        framesLeft = framesRead;
                    }
                } else {
                    framesRead = framesLeft;
                    data->position += static_cast<int>(framesRead);
                }

                sf_readf_int(audioFile->getData(), bufferCursor, framesRead);

                bufferCursor += framesRead;

                framesLeft -= framesRead;
            }


            auto outputCursor = static_cast<int *>(output);

            if (audioFile->getInfo().channels == 1) {
                for (unsigned long i = 0; i < stereoFrameCount; ++i) {
                    *outputCursor += static_cast<int>(0.5 * outputBuffer[i]);
                    ++outputCursor;
                    *outputCursor += static_cast<int>(0.5 * outputBuffer[i]);
                    ++outputCursor;
                }
            } else {
                for (unsigned long i = 0; i < stereoFrameCount; ++i) {
                    *outputCursor += static_cast<int>(0.5 * outputBuffer[i]);
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

void StreamHandler::processEvent(
    AudioEventType audioEventType,
    AudioFile * audioFile,
    bool loop
) {
    std::lock_guard<std::mutex> lock(dataMutex);

    switch (audioEventType) {
        case start:
            for (const auto & playback: data) {
                if (playback->audioFile == audioFile) {
                    return;
                }
            }

            if (Pa_IsStreamStopped(stream)) {
                PaError err = Pa_StartStream(stream);
                if (err != paNoError) {
                    stringstream error;
                    error << "Failed to start stream: " << Pa_GetErrorText(err);
                    throw std::runtime_error(error.str());
                }
            }

            data.push_back(std::make_unique<Playback>(Playback{
                audioFile,
                0,
                loop
            }));

            break;
        case stop: {
            PaError err = Pa_StopStream(stream);
            if (err != paNoError && err != paStreamIsStopped) {
                stringstream error;
                error << "Failed to stop stream: " << Pa_GetErrorText(err);
                throw std::runtime_error(error.str());
            }
        }
            data.clear();
            break;
    }
}

StreamHandler::StreamHandler()
    : stream() {
#if defined (__linux__)
    putenv(const_cast<char *>("PULSE_LATENCY_MSEC=10"));
#endif

    PaError errorCode = Pa_Initialize();
    if (errorCode != paNoError) {
        stringstream error;
        error << "Failed to initialize PortAudio: " << Pa_GetErrorText(errorCode);
        throw std::runtime_error(error.str());
    }

    PaStreamParameters outputParameters;

    PaDeviceIndex defaultDevice = Pa_GetDefaultOutputDevice();
    if (defaultDevice == paNoDevice) {
        Pa_Terminate();
        throw std::runtime_error("No default output device found");
    }

    outputParameters.device = defaultDevice;
    outputParameters.channelCount = CHANNEL_COUNT;
    outputParameters.sampleFormat = paInt32;
    outputParameters.suggestedLatency = 0.01;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    errorCode = Pa_OpenStream(
        &stream,
        NO_INPUT,
        &outputParameters,
        SAMPLE_RATE,
        paFramesPerBufferUnspecified,
        paNoFlag,
        &PortAudioCallback,
        this);

    if (errorCode != paNoError) {
        Pa_Terminate();

        stringstream error;
        error << "Unable to open stream for output: " << Pa_GetErrorText(errorCode);
        throw std::runtime_error(error.str());
    }
}

StreamHandler::~StreamHandler() {
    Pa_CloseStream(stream); {
        std::lock_guard<std::mutex> lock(dataMutex);
        data.clear();
    }
    Pa_Terminate();
}
