#include "KSAudioPlay.h"
#include <mutex>
class CXAudioPlay :public KSAudioPlay
{
public:
    std::mutex mux;
    
    virtual long long GetNoPlayMs()
    {
        
        return 1;
    }
    
    void Clear()
    {
        
    }
    virtual void Close()
    {
        
    }
    virtual bool Open()
    {
        
        return false;
    }
    void SetPause(bool isPause)
    {
        
    }
    virtual bool Write(const unsigned char *data, int datasize)
    {
        
        return true;
    }
    
    virtual int GetFree()
    {
        return 0;
    }
};
KSAudioPlay *KSAudioPlay::Get()
{
    static CXAudioPlay play;
    return &play;
}

KSAudioPlay::KSAudioPlay()
{
}


KSAudioPlay::~KSAudioPlay()
{
}
