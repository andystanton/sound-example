#include "util.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <portaudio.h>

std::string util::getApplicationPathAndName()
{
    std::string fullPath;

#if defined (__APPLE__) && defined (__MACH__)

    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    pid = getpid();
    ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
    if (ret <= 0) {
        throw std::runtime_error("Unable to ascertain application path");
    } else {
        fullPath = pathbuf;
    }

#elif defined (__linux__)

    char buff[1024];
    ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        buff[len] = '\0';
        fullPath = buff;
    } else {
        throw std::runtime_error("Unable to ascertain application path");
    }

#else

    throw std::runtime_error("OS not supported for finding paths");

#endif

    return fullPath;
}

std::string util::getApplicationPath()
{
    std::string fullPath = getApplicationPathAndName();
    return fullPath.substr(0, fullPath.find_last_of('/'));
}

std::string util::getApplicationPath(const std::string & relativePath)
{
    std::string fullPath = getApplicationPathAndName();
    return fullPath.substr(0, fullPath.find_last_of('/')) + relativePath;
}


void util::wrapPortAudioCall(const std::string & description, const std::function<PaError()> & f)
{
    PaError error = f();
    if (error != paNoError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to execute PortAudio command " << description << " (" << error << ": " << Pa_GetErrorText(error) << ")";
        throw std::runtime_error(errorMessage.str());
    }
}

void util::wrapPortAudioCallOrTerminate(const std::string & description, const std::function<PaError()> & f)
{
    PaError error = f();
    if (error != paNoError) {
        std::stringstream errorMessage;
        errorMessage << "Unable to close PortAudio command " << description << " (" << error << ": " << Pa_GetErrorText(error) << ")";
        PaError terminateError = Pa_Terminate();
        if (terminateError != paNoError) {
            errorMessage << "\n" << "Unable to terminate PortAudio (" << terminateError << ": " << Pa_GetErrorText(terminateError) << ")";
        }
        throw std::runtime_error(errorMessage.str());
    }
}

#if !defined (_WIN32) && !defined (_WIN64)

static struct termios oldTermios;

void cleanupTermios()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldTermios);
}

void initTerminal()
{
    tcgetattr(STDIN_FILENO, &oldTermios);

    struct termios newTermios = oldTermios;

    newTermios.c_lflag &= ~ICANON;
    newTermios.c_lflag &= ~ECHO;
    newTermios.c_cc[VMIN] = 0;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newTermios) == 0) {
        atexit(cleanupTermios);
    } else {
        std::cerr << "Unable to set terminal mode" << std::endl;
        exit(1);
    }
}

int kbhit()
{
    struct timeval tv {};
    fd_set rdfs {};

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, nullptr, nullptr, &tv);

    return FD_ISSET(STDIN_FILENO, &rdfs);
}

#endif
