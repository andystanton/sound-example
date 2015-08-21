#pragma once

#include "AudioFile.hpp"
#include "util.hpp"

#include <map>
#include <memory>
#include <sstream>
#include <string>

using std::map;
using std::string;
using std::stringstream;
using std::unique_ptr;

class FileHandler
{
public:
    FileHandler();
    ~FileHandler();

    bool containsSound(string filename);
    AudioFile & getSound(string filename);
private:
    map<string, AudioFile> sounds;
};
