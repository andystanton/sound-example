#pragma once

#include "StreamHandler.hpp"
#include "FileHandler.hpp"

class AudioPlayer
{
public:
    AudioPlayer();

    void play(string soundfile);
    void loop(string soundfile);
    void stop();

private:
    FileHandler fileHandler;
    StreamHandler streamHandler;
};
