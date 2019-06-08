#include "AudioPlayer.hpp"

void AudioPlayer::play(const std::string & soundfile)
{
    streamHandler.processEvent(AudioEventType::start, &fileHandler.getSound(soundfile));
}

void AudioPlayer::loop(const std::string & soundfile)
{
    streamHandler.processEvent(AudioEventType::start, &fileHandler.getSound(soundfile), true);
}

void AudioPlayer::stop()
{
    streamHandler.processEvent(AudioEventType::stop);
}
