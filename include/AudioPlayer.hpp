#pragma once

#include "StreamHandler.hpp"
#include "FileHandler.hpp"

class AudioPlayer
{
public:
    void play(const std::string & soundfile);
    void loop(const std::string & soundfile);
    void stop();

private:
    FileHandler fileHandler;
    StreamHandler streamHandler;
};
