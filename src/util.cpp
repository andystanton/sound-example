#include "util.hpp"

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

#if !defined (_WIN32) && !defined (_WIN64)

void changemode(int dir)
{
    static struct termios oldt, newt;

    if (dir == 1) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
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
