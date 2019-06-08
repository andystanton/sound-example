#include "AudioPlayer.hpp"
#include "util.hpp"

#include <iostream>
#include <stdexcept>

using std::endl;
using std::cout;
using std::cerr;

const int CHANNEL_COUNT = 2;

int portAudioCallback(
    const void * input,
    void * output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo * paTimeInfo,
    PaStreamCallbackFlags statusFlags,
    void * userData
)
{
    memset(output, 0, CHANNEL_COUNT * frameCount * sizeof(float));
    return paContinue;
}

int main(int argc, char ** argv)
{
    try {
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
            nullptr
        );

        cout << "Playback with PortAudio and libsndfile" << endl << endl;

        cout << "Options: " << endl;
        cout << " o: stereo sound" << endl;
        cout << " p: mono sound" << endl;
        cout << " l: loop sound" << endl;
        cout << " k: stop all sounds" << endl << endl;

        cout << "Press 'Q' to quit" << endl;

        initTerminal();

        bool done = false;
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
        stream = nullptr;
    } catch (const std::runtime_error & error) {
        cerr << "Caught error: " << error.what() << endl;
        return 1;
    }

    return 0;
}
