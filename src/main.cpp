#include "AudioPlayer.hpp"
#include "util.hpp"

#include <iostream>
#include <stdexcept>

using std::endl;
using std::cout;
using std::cerr;

int main(int argc, char ** argv)
{
    try {
        AudioPlayer player;

        cout << "Playback with PortAudio and libsndfile" << endl << endl;

        cout << "Options: " << endl;
        cout << " O: stereo sound" << endl;
        cout << " P: mono sound" << endl;
        cout << " L: loop sound" << endl;
        cout << " K: stop all sounds" << endl << endl;

        cout << "Press 'Q' to quit" << endl;

        int ch;
        changemode(1);

        for (bool done = false; !done; ) {
            if (!kbhit()) {
                ch = getchar();
                switch (ch) {
                    case 'q':
                    case 'Q':
                        done = true;
                        break;
                    case 'o':
                    case 'O':
                        player.play("Powerup14.wav");
                        break;
                    case 'p':
                    case 'P':
                        player.play("Powerup47.wav");
                        break;
                    case 'l':
                    case 'L':
                        player.loop("loop.wav");
                        break;
                    case 'k':
                    case 'K':
                        player.stop();
                        break;
                    default:
                        break;
                }
            }
        }
        changemode(0);
    } catch (const std::runtime_error & error) {
        cerr << "Caught error: " << error.what() << endl;
        return 1;
    }

    return 0;
}
