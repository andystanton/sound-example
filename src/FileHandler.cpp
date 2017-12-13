#include "FileHandler.hpp"

#include "util.hpp"

#include <sstream>

FileHandler::FileHandler()
        : sounds()
{

}

FileHandler::~FileHandler()
{
    for (auto & entry : sounds) {
        sf_close(entry.second.data);
    }
}

bool FileHandler::containsSound(const std::string & filename)
{
    return sounds.find(filename) != sounds.end();
}

AudioFile & FileHandler::getSound(const std::string & filename)
{
    if (!containsSound(filename)) {
        std::string fullFilename = util::getApplicationPath() + "/sounds/" + filename;
        SF_INFO info {};
        info.format = 0;
        SNDFILE * audioFile = sf_open(fullFilename.c_str(), SFM_READ, &info);

        AudioFile sound {
                audioFile,
                info
        };

        if (!audioFile) {
            std::stringstream error;
            error << "Unable to open audio file '" << filename
                  << "' with full filename '" << fullFilename << "'";
            throw error.str();
        }
        sounds[filename] = sound;
    }
    return sounds[filename];
}
