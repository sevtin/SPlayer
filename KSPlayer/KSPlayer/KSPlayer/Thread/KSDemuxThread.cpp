#include "KSDemuxThread.h"
#include <iostream>
#include <thread>
#include "KSDemux.h"
#include "KSVideoThread.h"
#include "KSAudioThread.h"
#include "KSDecode.h"
extern "C" {
#include "libavformat/avformat.h"
}
using namespace std;

void KSDemuxThread::Clear()
{
    mux.lock();
    if (demux)demux->Clear();
    if (video_thread) video_thread->Clear();
    if (audio_thread) audio_thread->Clear();
    mux.unlock();
}

void KSDemuxThread::Seek(double pos) {
    //清理缓存
    Clear();
    
    mux.lock();
    bool status = this->isPause;
    mux.unlock();
    
    //暂停
    SetPause(true);
    
    mux.lock();
    if (demux) {
        demux->Seek(pos);
    }
    
    //实际要显示的位置pts
    long long seekPts = pos*demux->total_ms;
    while (!isExit) {
        AVPacket *pkt = demux->ReadVideo();
        if (!pkt) {
            break;
        }
        //如果解码到seekPts
        if (video_thread->RepaintPts(pkt, seekPts)) {
            this->pts = seekPts;
            break;
        }
    }
    
    mux.unlock();
    //seek是非暂停状态
    if(!status) {
        SetPause(false);
    }
}

void KSDemuxThread::SetPause(bool isPause) {
    mux.lock();
    this->isPause = isPause;
    if (audio_thread) audio_thread->SetPause(isPause);
    if (video_thread) video_thread->SetPause(isPause);
    mux.unlock();
}

void KSDemuxThread::Runloop(){
    while (!isExit) {
        mux.lock();
        if (isPause) {
            mux.unlock();
            msleep(10);
            continue;
        }
        if (!demux) {
            mux.unlock();
            msleep(10);
            continue;
        }
        
        //音视频同步
        if (video_thread && audio_thread) {
            pts = audio_thread->pts;
            video_thread->synpts = audio_thread->pts;
        }
        
        AVPacket *pkt = demux->Read();
        if (!pkt)  {
            mux.unlock();
            msleep(10);
            continue;
        }
        //判断数据是音频
        if (demux->IsAudio(pkt)) {
            //if(audio_thread)audio_thread->Push(pkt);
        }
        else {
            if (video_thread)video_thread->Push(pkt);
        }
        mux.unlock();
        msleep(5);
    }
}

bool KSDemuxThread::Open(const char *url, KSProtocol *call) {
    if (url == 0 || url[0] == '\0')
        return false;
    
    mux.lock();
    if (!demux) demux = new KSDemux();
    if (!video_thread) video_thread = new KSVideoThread();
    if (!audio_thread) audio_thread = new KSAudioThread();
    
    //打开解封装
    bool ret = demux->Open(url);
    if (!ret) {
        mux.unlock();
        cout << "demux->Open(url) failed!" << endl;
        return false;
    }
    //打开视频解码器和处理线程
    if (!video_thread->Open(demux->CopyVPara(), call, demux->width, demux->height)){
        ret = false;
        cout << "video_thread->Open failed!" << endl;
    }
    //打开音频解码器和处理线程
    if (!audio_thread->Open(demux->CopyAPara(), demux->sample_rate, demux->channels)) {
        ret = false;
        cout << "audio_thread->Open failed!" << endl;
    }
    total_ms = demux->total_ms;
    mux.unlock();
    
    cout << "KSDemuxThread::Open " << ret << endl;
    return ret;
}

//关闭线程清理资源
void KSDemuxThread::Close() {
    isExit = true;
    wait1();
    if (video_thread) video_thread->Close();
    if (audio_thread) audio_thread->Close();
    mux.lock();
    delete video_thread;
    delete audio_thread;
    video_thread = NULL;
    audio_thread = NULL;
    mux.unlock();
}

void StartDemuxThread(KSDemuxThread *demux_thread) {
    demux_thread->Runloop();
}

void StartVideoThread(KSVideoThread *video_thread) {
    video_thread->Runloop();
}
//启动所有线程
void KSDemuxThread::Start()
{
    //mux.lock();
    if (!demux) demux = new KSDemux();
    if (!video_thread) video_thread = new KSVideoThread();
    if (!audio_thread) audio_thread = new KSAudioThread();

    std::thread threads[2];
    threads[0] = std::thread(StartDemuxThread,this);
    threads[1] = std::thread(StartVideoThread,video_thread);
    std::cout << "Done spawning threads. Now waiting for them to join:\n";
    for (auto &thread : threads) {
        thread.join();
    }
    std::cout << "All threads joined!\n";
}


KSDemuxThread::KSDemuxThread()
{
}


KSDemuxThread::~KSDemuxThread()
{
    isExit = true;
    wait1();
}
