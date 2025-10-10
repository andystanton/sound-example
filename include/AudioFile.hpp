#pragma once

#include "sndfile.h"
#include <memory>

class AudioFile {
public:
    AudioFile(
        SNDFILE * file,
        const SF_INFO & fileInfo
    )
        : data(file, &sf_close)
        , info(fileInfo)
    {
    }

    AudioFile(const AudioFile &) = delete;

    AudioFile &operator=(const AudioFile &) = delete;

    AudioFile(AudioFile &&) = default;

    AudioFile &operator=(AudioFile &&) = default;

    SNDFILE *getData() const { return data.get(); }
    const SF_INFO &getInfo() const { return info; }

private:
    std::unique_ptr<SNDFILE, decltype(&sf_close)> data;
    SF_INFO info;
};
