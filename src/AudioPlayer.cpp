#include "AudioPlayer.hpp"

using std::string;

AudioPlayer::AudioPlayer() = default;

void AudioPlayer::play(const string& soundfile)
{
        streamHandler.processEvent(AudioEventType::start, fileHandler.getSound(soundfile));
}

void AudioPlayer::loop(const string& soundfile)
{
        streamHandler.processEvent(AudioEventType::start, fileHandler.getSound(soundfile), true);

}

void AudioPlayer::stop()
{
        streamHandler.processEvent(AudioEventType::stop);
}
