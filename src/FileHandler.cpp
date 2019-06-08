#include "FileHandler.hpp"

#include "util.hpp"

#include <stdexcept>

FileHandler::~FileHandler()
{
    for (auto & [k, v] : sounds) {
        sf_close(v.data);
    }
    sounds.clear();
}

bool FileHandler::containsSound(const std::string & filename)
{
    bool found = sounds.find(filename) != sounds.end();
    return found;
}

AudioFile & FileHandler::getSound(const std::string & filename)
{
    if (!containsSound(filename)) {
        std::string fullFilename = util::getApplicationPath("/sounds/" + filename);
        SF_INFO info {
            .frames = 0
        };
        SNDFILE * audioFile = sf_open(fullFilename.c_str(), SFM_READ, &info);

        AudioFile sound {
            audioFile,
            info
        };

        if (!audioFile) {
            throw std::runtime_error("Unable to open audio file '" + filename + "' with full filename '" + fullFilename + "'");
        }
        sounds[filename] = sound;
    }
    auto & file = sounds[filename];
    return file;
}
