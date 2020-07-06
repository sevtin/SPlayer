#include "KSAudioThread.h"
#include <iostream>
#include "KSDecode.h"
#include "KSAudioPlay.h"
#include "KSResample.h"

using namespace std;

void KSAudioThread::Clear() {
    KSThread::Clear();
    mux.lock();
    if (audio_play) audio_play->Clear();
    mux.unlock();
}

//停止线程，清理资源
void KSAudioThread::Close()  {
    KSThread::Close();
    if (res) {
        res->Close();
        amux.lock();
        delete res;
        res = NULL;
        amux.unlock();
    }
    if (audio_play) {
        audio_play->Close();
        amux.lock();
        audio_play = NULL;
        amux.unlock();
    }
}
bool KSAudioThread::Open(AVCodecParameters *para,int sample_rate, int channels)
{
    if (!para)return false;
    Clear();
    
    amux.lock();
    pts = 0;
    bool re = true;
    if (!res->Open(para, false)) {
        cout << "KSResample open failed!" << endl;
        re = false;
    }
    audio_play->sample_rate = sample_rate;
    audio_play->channels = channels;
    if (!audio_play->Open()) {
        re = false;
        cout << "XAudioPlay open failed!" << endl;
    }
    if (!decode->Open(para)) {
        cout << "audio XDecode open failed!" << endl;
        re = false;
    }
    amux.unlock();
    cout << "KSAudioThread::Open :" << re << endl;
    return re;
}

void KSAudioThread::SetPause(bool isPause)
{
    //amux.lock();
    this->isPause = isPause;
    if (audio_play)
        audio_play->SetPause(isPause);
    //amux.unlock();
}

void KSAudioThread::Runloop()
{
    unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
    while (!isExit)
    {
        amux.lock();
        if (isPause) {
            amux.unlock();
            msleep(10);
            continue;
        }
        
        //没有数据
        //if (packs.empty() || !decode || !res || !audio_play)
        //{
        //	mux.unlock();
        //	msleep(1);
        //	continue;
        //}
        
        //AVPacket *pkt = packs.front();
        //packs.pop_front();
        AVPacket *pkt = Pop();
        bool re = decode->Send(pkt);
        if (!re)
        {
            amux.unlock();
            msleep(5);
            continue;
        }
        //一次send 多次recv
        while (!isExit)
        {
            AVFrame * frame = decode->Receive();
            if (!frame) break;
            
            //减去缓冲中未播放的时间
            pts = decode->pts - audio_play->GetNoPlayMs();
            
            //cout << "audio pts = " << pts << endl;
            
            //重采样 
            int size = res->Resample(frame, pcm);
            //播放音频
            while (!isExit)
            {
                if (size <= 0)break;
                //缓冲未播完，空间不够
                if (audio_play->GetFree() < size || isPause)
                {
                    msleep(5);
                    continue;
                }
                audio_play->Write(pcm, size);
                break;
            }
        }
        amux.unlock();
    }
    delete pcm;
}

KSAudioThread::KSAudioThread()
{
    if (!res) res = new KSResample();
    if (!audio_play) audio_play = KSAudioPlay::Get();
}


KSAudioThread::~KSAudioThread()
{
    //等待线程退出
    isExit = true;
    wait1();
}
