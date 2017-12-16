#pragma once

#include "AudioFile.hpp"

#include <map>
#include <string>

class FileHandler
{
public:
    ~FileHandler();

    bool containsSound(const std::string & filename);
    AudioFile & getSound(const std::string & filename);
private:
    std::map<std::string, AudioFile> sounds;
};
