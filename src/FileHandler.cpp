#include "FileHandler.hpp"
#include <stdexcept>
#include <sstream>

using std::string;
using std::stringstream;

bool FileHandler::containsSound(const string& filename) const
{
        return sounds.find(filename) != sounds.end();
}

AudioFile* FileHandler::getSound(const string& filename)
{
        if (!containsSound(filename)) {
                string fullFilename = util::getApplicationPath() + "/sounds/" + filename;
                SF_INFO info;
                info.format = 0;
                SNDFILE* rawFile = sf_open(fullFilename.c_str(), SFM_READ, &info);

                if (!rawFile)
                {
                        stringstream error;
                        error << "Unable to open audio file '"
                              << filename << "' with full filename '"
                              << fullFilename << "'";
                        throw std::runtime_error(error.str());
                }

                sounds.emplace(filename, AudioFile(rawFile, info));
        }
        return &sounds.at(filename);
}
