#include "AudioPlayer.hpp"
#include "util.hpp"

#include <iostream>
#include <stdexcept>
#include <thread>

using std::endl;
using std::cout;
using std::cerr;

const int CHANNEL_COUNT = 2;

typedef struct
{
    SF_INFO info;
    SNDFILE * audioFile;
    int position;
    bool done;
} NowPlaying;

enum PlaybackVector{PlaybackNone, PlaybackA, PlaybackB};

typedef struct
{
    std::vector<NowPlaying> a;
    std::vector<NowPlaying> b;
    volatile PlaybackVector targetPlaybackVector;
    volatile PlaybackVector actualPlaybackVector;
} PlaybackItems;

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
    auto * playbackItems = (PlaybackItems*) userData;

    std::vector<NowPlaying> * np;
    PlaybackVector target = playbackItems->targetPlaybackVector;
    PlaybackVector switchingTo = PlaybackNone;
    switch (target) {
        case PlaybackA:
            np = &playbackItems->a;
            switchingTo = PlaybackA;
            break;
        case PlaybackB:
            np = &playbackItems->b;
            switchingTo = PlaybackB;
            break;
        case PlaybackNone:
        default:
            np = nullptr;
            switchingTo = PlaybackNone;
            break;
    }

    if (np && !np->empty()) {
        auto it = np->begin();
        while (it != np->end()) {
            NowPlaying & data = *it;

            float outputBuffer[stereoFrameCount];
            float * bufferCursor = outputBuffer;

            unsigned long framesLeft = frameCount;
            unsigned long framesRead;

            while (framesLeft > 0) {
                sf_seek(data.audioFile, data.position, SEEK_SET);

                if (framesLeft > (data.info.frames - data.position)) {
                    framesRead = (unsigned int) (data.info.frames - data.position);
                    framesLeft = framesRead;
                    data.done = true;
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
            ++it;
        }
    }
    playbackItems->actualPlaybackVector = switchingTo;
    if (switchingTo == PlaybackNone) {
        return paAbort;
    } else {
        return paContinue;
    }
}


int main(int argc, char ** argv)
{
    SF_INFO powerup47Info;
    SNDFILE * powerup47Data = sf_open(util::getApplicationPath("/sounds/Powerup47.wav").c_str(), SFM_READ, &powerup47Info);

    SF_INFO powerup14Info;
    SNDFILE * powerup14Data = sf_open(util::getApplicationPath("/sounds/Powerup14.wav").c_str(), SFM_READ, &powerup14Info);

    PlaybackItems playbackItems = {
        .a = {},
        .b = {},
        .targetPlaybackVector = PlaybackNone,
        .actualPlaybackVector = PlaybackNone,
    };

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
        &playbackItems
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
        PlaybackVector target = playbackItems.targetPlaybackVector;
        PlaybackVector actual = playbackItems.actualPlaybackVector;
        PlaybackVector switchingTo = target;

        if (target == actual) {
            // TODO: make this more granular - not blocking things like quitting

            switch (actual) {
                case PlaybackA:
                    if (std::find_if(playbackItems.a.begin(), playbackItems.a.end(), [](const NowPlaying & item) { return item.done; }) != playbackItems.a.end()) {
                        playbackItems.b.clear();
                        playbackItems.b.resize(playbackItems.a.size());
                        switchingTo = PlaybackB;
                        std::remove_copy_if(playbackItems.a.begin(), playbackItems.a.end(), playbackItems.b.begin(), [](const NowPlaying & item) { return item.done; });
                    }
                    break;
                case PlaybackB:
                    if (std::find_if(playbackItems.b.begin(), playbackItems.b.end(), [](const NowPlaying & item) { return item.done; }) != playbackItems.b.end()) {
                        playbackItems.a.clear();
                        playbackItems.a.resize(playbackItems.b.size());
                        switchingTo = PlaybackA;
                        std::remove_copy_if(playbackItems.b.begin(), playbackItems.b.end(), playbackItems.a.begin(), [](const NowPlaying & item) { return item.done; });
                    }
                    break;
                default:
                    break;
            }

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
                        switch (actual) {
                            case PlaybackA:
                                 if (switchingTo == target) {
                                     switchingTo = PlaybackB;
                                     playbackItems.b = playbackItems.a;
                                 }
                                 playbackItems.b.push_back({
                                     .audioFile = powerup47Data,
                                     .info = powerup47Info,
                                     .position = 0,
                                     .done = false,
                                 });
                                break;
                            case PlaybackB:
                            case PlaybackNone:
                                if (switchingTo == target) {
                                    switchingTo = PlaybackA;
                                    playbackItems.a = playbackItems.b;
                                }
                                playbackItems.a.push_back({
                                    .audioFile = powerup47Data,
                                    .info = powerup47Info,
                                    .position = 0,
                                    .done = false,
                                });
                                break;
                            default:
                                break;
                        }
                        //                        player.play("Powerup14.wav");
                        break;
                    case 'p':
                    case 'P':
                        if (Pa_IsStreamStopped(stream)) {
                            Pa_StartStream(stream);
                        }
                        switch (actual) {
                            case PlaybackA:
                                if (switchingTo == target) {
                                    switchingTo = PlaybackB;
                                    playbackItems.b = playbackItems.a;
                                }
                                playbackItems.b.push_back({
                                    .audioFile = powerup14Data,
                                    .info = powerup14Info,
                                    .position = 0,
                                    .done = false,
                                });
                                break;
                            case PlaybackB:
                            case PlaybackNone:
                                if (switchingTo == target) {
                                    switchingTo = PlaybackA;
                                    playbackItems.a = playbackItems.b;
                                }
                                playbackItems.a.push_back({
                                    .audioFile = powerup14Data,
                                    .info = powerup14Info,
                                    .position = 0,
                                    .done = false,
                                });
                                break;
                            default:
                                break;
                        }
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
            playbackItems.targetPlaybackVector = switchingTo;

        } // otherwise callback hasn't caught up yet


    }
    //        player.stop();

    if (!Pa_IsStreamStopped(stream)) {
        Pa_StopStream(stream);
        //            playingSounds.clear();
    }
    Pa_CloseStream(stream);
    Pa_Terminate();

    sf_close(powerup14Data);
    sf_close(powerup47Data);

    return 0;
}
