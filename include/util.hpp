#pragma once

#include <string>

#if defined (__APPLE__) && defined (__MACH__)

#include <cstdio>
#include <dirent.h>
#include <libproc.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#elif defined (__linux__)

#include <cstdio>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#endif

namespace util
{
    std::string getApplicationPathAndName();
    std::string getApplicationPath();
    std::string getApplicationPath(const std::string &);
}

#if !defined (_WIN32) && !defined (_WIN64)

void initTerminal();
int kbhit();

#endif
