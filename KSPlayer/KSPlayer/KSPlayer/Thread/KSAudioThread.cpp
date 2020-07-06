#include "KSAudioThread.h"
#include <iostream>
#include "KSDecode.h"
#include "KSAudioPlay.h"
#include "KSResample.h"

using namespace std;

void KSAudioThread::Clear()
{
    KSThread::Clear();
    mux.lock();
    if (ap) ap->Clear();
    mux.unlock();
}
//停止线程，清理资源
void KSAudioThread::Close()
{
    KSThread::Close();
    if (res)
    {
        res->Close();
        amux.lock();
        delete res;
        res = NULL;
        amux.unlock();
    }
    if (ap)
    {
        ap->Close();
        amux.lock();
        ap = NULL;
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
    if (!res->Open(para, false))
    {
        cout << "KSResample open failed!" << endl;
        re = false;
    }
    ap->sample_rate = sample_rate;
    ap->channels = channels;
    if (!ap->Open())
    {
        re = false;
        cout << "XAudioPlay open failed!" << endl;
    }
    if (!decode->Open(para))
    {
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
    if (ap)
        ap->SetPause(isPause);
    //amux.unlock();
}

void KSAudioThread::run()
{
    unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
    while (!isExit)
    {
        amux.lock();
        if (isPause)
        {
            amux.unlock();
            msleep(5);
            continue;
        }
        
        //没有数据
        //if (packs.empty() || !decode || !res || !ap)
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
            msleep(1);
            continue;
        }
        //一次send 多次recv
        while (!isExit)
        {
            AVFrame * frame = decode->Receive();
            if (!frame) break;
            
            //减去缓冲中未播放的时间
            pts = decode->pts - ap->GetNoPlayMs();
            
            //cout << "audio pts = " << pts << endl;
            
            //重采样 
            int size = res->Resample(frame, pcm);
            //播放音频
            while (!isExit)
            {
                if (size <= 0)break;
                //缓冲未播完，空间不够
                if (ap->GetFree() < size || isPause)
                {
                    msleep(1);
                    continue;
                }
                ap->Write(pcm, size);
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
    if (!ap) ap = KSAudioPlay::Get();
}


KSAudioThread::~KSAudioThread()
{
    //等待线程退出
    isExit = true;
    wait1();
}
