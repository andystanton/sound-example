#include "FileHandler.hpp"

FileHandler::FileHandler()
        : sounds()
{

}

FileHandler::~FileHandler()
{
        for (auto entry : sounds)
        {

        }
}

bool FileHandler::containsSound(string filename)
{
        return sounds.find(filename) != sounds.end();
}

AudioFile & FileHandler::getSound(string filename)
{
        if (!containsSound(filename)) {
                string fullFilename = util::getApplicationPath() + "/sounds/" + filename;
                SF_INFO info;
                info.format = 0;
                SNDFILE * audioFile = sf_open(fullFilename.c_str(), SFM_READ, &info);

                array<int16_t, 4096> read_buf;
                size_t read_size;
                vector<uint16_t> vdata;

                while((read_size = sf_read_short(audioFile, read_buf.data(), read_buf.size())) != 0) {
                        vdata.insert(vdata.end(), read_buf.begin(), read_buf.begin() + read_size);
                }
                sf_close(audioFile);


                AudioFile sound {
                    info,
                    vdata
                };

                if (!audioFile)
                {
                        stringstream error;
                        error << "Unable to open audio file '"
                              << filename << "' with full filename '"
                              << fullFilename << "'";
                        throw error.str();
                }
                sounds[filename] = sound;
        }
        return sounds[filename];
}
