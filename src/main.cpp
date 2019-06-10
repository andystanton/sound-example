#include "AudioPlayer.hpp"
#include "util.hpp"

#include <iostream>
#include <stdexcept>

using std::endl;
using std::cout;
using std::cerr;

const int CHANNEL_COUNT = 2;


typedef struct
{
    SF_INFO info;
    SNDFILE * audioFile;
    int position;
} NowPlaying;

int portAudioCallback(
    const void * input,
    void * output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo * paTimeInfo,
    PaStreamCallbackFlags statusFlags,
    void * userData
)
{
    auto stereoFrameCount = CHANNEL_COUNT * frameCount;
    memset(output, 0, stereoFrameCount * sizeof(float));
    auto * np = (std::vector<NowPlaying> *) userData;

    if (!np->empty()) {
        auto it = np->begin();
        while (it != np->end()) {
            NowPlaying & data = *it;

            float outputBuffer[stereoFrameCount];
            float * bufferCursor = outputBuffer;

            unsigned long framesLeft = frameCount;
            unsigned long framesRead;

            bool playbackEnded = false;
            while (framesLeft > 0) {
                sf_seek(data.audioFile, data.position, SEEK_SET);

                if (framesLeft > (data.info.frames - data.position)) {
                    framesRead = (unsigned int) (data.info.frames - data.position);
                    //                    if (data.loop) {
                    //                        data.position = 0; // TODO: should read from the beginning as well
                    //                    } else {
                    playbackEnded = true;
                    framesLeft = framesRead;
                    //                    }
                } else {
                    framesRead = framesLeft;
                    data.position += framesRead;
                }

                sf_readf_float(data.audioFile, bufferCursor, framesRead);

                bufferCursor += framesRead;

                framesLeft -= framesRead;
            }

            auto * outputCursor = (float *) output;
            if (data.info.channels == 1) {
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
                it = np->erase(it);
            } else {
                ++it;
            }
        }
    }

    return paContinue;
}


int main(int argc, char ** argv)
{
    std::string fullFilename = util::getApplicationPath("/sounds/Powerup47.wav");
    SF_INFO info;
    SNDFILE * audioFile = sf_open(fullFilename.c_str(), SFM_READ, &info);
    printf("channels: %d\n", info.channels);

    std::vector<NowPlaying> nowPlaying;

    const unsigned long SAMPLE_RATE = 44000;
    const PaStreamParameters * NO_INPUT = nullptr;
    PaStream * stream = nullptr;
#if defined (__linux__)
    putenv((char *) "PULSE_LATENCY_MSEC=10");
#endif

    Pa_Initialize();

    PaStreamParameters outputParameters {
        .device = Pa_GetDefaultOutputDevice(),
        .channelCount = CHANNEL_COUNT,
        .sampleFormat = paFloat32,
        .suggestedLatency = 0.01,
        .hostApiSpecificStreamInfo = nullptr,
    };

    Pa_OpenStream(
        &stream,
        NO_INPUT,
        &outputParameters,
        SAMPLE_RATE,
        paFramesPerBufferUnspecified,
        paNoFlag,
        portAudioCallback,
        &nowPlaying
    );

    cout << "Playback with PortAudio and libsndfile" << endl << endl;

    cout << "Options: " << endl;
    cout << " o: stereo sound" << endl;
    cout << " p: mono sound" << endl;
    cout << " l: loop sound" << endl;
    cout << " k: stop all sounds" << endl << endl;

    cout << "Press 'q' to quit" << endl;

    initTerminal();

    bool done = false;
    bool started = false;
    while (!done) {
        if (kbhit()) {
            int ch = getchar();
            switch (ch) {
                case 'q':
                case 'Q':
                    done = true;
                    break;
                case 'o':
                case 'O':
                    if (Pa_IsStreamStopped(stream)) {
                        Pa_StartStream(stream);
                    }
                    nowPlaying.push_back({
                        .audioFile = audioFile,
                        .info = info,
                        .position = 0,
                    });
                    //                        player.play("Powerup14.wav");
                    break;
                case 'p':
                case 'P':
                    //                        player.play("Powerup47.wav");
                    break;
                case 'l':
                case 'L':
                    //                        player.loop("loop.wav");
                    break;
                case 'k':
                case 'K':
                    //                        player.stop();
                    if (!Pa_IsStreamStopped(stream)) {
                        Pa_StopStream(stream);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    //        player.stop();

    if (!Pa_IsStreamStopped(stream)) {
        Pa_StopStream(stream);
        //            playingSounds.clear();
    }
    Pa_CloseStream(stream);
    Pa_Terminate();

    sf_close(audioFile);

    return 0;
}
