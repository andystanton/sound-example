#pragma once

#include "AudioFile.hpp"
#include "util.hpp"

#include <map>
#include <string>

class FileHandler {
public:
    FileHandler() = default;

    bool containsSound(const std::string & filename) const;

    AudioFile *getSound(const std::string & filename);

private:
    std::map<std::string, AudioFile> sounds;
};
