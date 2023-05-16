#include "../audio.h"

extern "C" u32 AudioCreateBuffer(u32 numChannels, u32 sampleRate, u32 numSamples, short* pcmData) {
    return 0;
}

extern "C" void AudioDestroyBuffer(u32 buffer) {

}

extern "C" u32 AudioCreateBus() {
    return 0;
}

extern "C" void AudioDestroyBus(u32 bus) {

}

extern "C" u32 AudioPlay2D(u32 buffer, u32 bus, bool looping, float volume, float pan) {
    return 0;
}

extern "C" u32 AudioPlay3D(u32 buffer, u32 bus, bool looping, float volume, float px, float py, float pz, float minattenuation, float maxattenuation) {
    return 0;
}

extern "C" void AudioStop(u32 soundId) {

}

extern "C" void AudioSetListener(float px, float py, float pz, float upx, float upy, float upz, float fwdx, float fwdy, float fwdz) {

}

extern "C" void AudioSetVolume(u32 soundOrBus, float volume) {

}

extern "C" void AudioSetPan(u32 soundOrBus, float pan) {

}

extern "C" void AudioSetPosition(u32 sound, float px, float py, float pz) {

}

extern "C" void AudioSetAttenuation(u32 sound, float minatten, float maxatten) {

}