#include "AudioPlayer.hpp"

AudioPlayer::AudioPlayer()
        : fileHandler()
        , streamHandler()
{

}

void AudioPlayer::play(string soundfile)
{
        streamHandler.processEvent(AudioEventType::start, &fileHandler.getSound(soundfile));
}

void AudioPlayer::loop(string soundfile)
{
        streamHandler.processEvent(AudioEventType::start, &fileHandler.getSound(soundfile), true);
}

void AudioPlayer::stop()
{
        streamHandler.processEvent(AudioEventType::stop);
}
