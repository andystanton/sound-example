#pragma once

#include "sndfile.h"

#include <array>
#include <vector>

using std::array;
using std::vector;

struct AudioFile
{
        SF_INFO info;
        vector<uint16_t> vdata;
};
