#pragma once
#include <mutex>
#include <list>
#include "XDecodeThread.h"
#include "KSThread.h"
struct AVCodecParameters;
class KSAudioPlay;
class KSResample;

class KSAudioThread:public KSThread {
public:
    //当前音频播放的pts
    long long pts = 0;
    //打开，不管成功与否都清理
    virtual bool Open(AVCodecParameters *para,int sampleRate,int channels);
    
    //停止线程，清理资源
    virtual void Close();
    
    virtual void Clear();
    void Runloop();
    KSAudioThread();
    virtual ~KSAudioThread();
    void SetPause(bool isPause);
    bool isPause = false;
protected:
    std::mutex amux;
    KSAudioPlay *audio_play = 0;
    KSResample *res = 0;
};

