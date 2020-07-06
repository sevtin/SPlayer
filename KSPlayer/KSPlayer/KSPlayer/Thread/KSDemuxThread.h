#pragma once
#include "KSProtocol.h"
#include "KSThread.h"
#include <mutex>
class KSDemux;
class KSVideoThread;
class KSAudioThread;

class KSDemuxThread: public KSThread
{
public:
    //创建对象并打开
    virtual bool Open(const char *url, KSProtocol *call);
    
    //启动所有线程
    virtual void Start();
    
    //关闭线程清理资源
    virtual void Close();
    virtual void Clear();
    
    virtual void Seek(double pos);
    
    void run();
    KSDemuxThread();
    virtual ~KSDemuxThread();
    bool isExit = false;
    long long pts = 0;
    long long totalMs = 0;
    void SetPause(bool isPause);
    bool isPause = false;
protected:
    std::mutex mux;
    KSDemux *demux = 0;
    KSVideoThread *vt = 0;
    KSAudioThread *at = 0;
};
