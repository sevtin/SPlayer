//
//  KSThread.cpp
//  KSPlayer
//
//  Created by saeipi on 2020/7/6.
//  Copyright © 2020 saeipi. All rights reserved.
//

#include "KSThread.h"
#include <thread>
#include "KSDecode.h"

void KSThread::msleep(int ms) {
    //std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void KSThread::wait1() {
    
}

KSThread::KSThread() {
    decode = new KSDecode();
}

KSThread::~KSThread() {
    
}



//清理资源，停止线程
void KSThread::Close() {
    Clear();
    
    //等待线程退出
    isExit = true;
    wait1();
    decode->Close();
    
    mux.lock();
    delete decode;
    decode = NULL;
    mux.unlock();
}
void KSThread::Clear() {
    mux.lock();
    decode->Clear();
    while (!packs.empty())
    {
        AVPacket *pkt = packs.front();
        KSFreePacket(&pkt);
        packs.pop_front();
    }
    
    mux.unlock();
}


//取出一帧数据，并出栈，如果没有返回NULL
AVPacket *KSThread::Pop() {
    mux.lock();
    if (packs.empty())
    {
        mux.unlock();
        return NULL;
    }
    AVPacket *pkt = packs.front();
    packs.pop_front();
    mux.unlock();
    return pkt;
}

void KSThread::Push(AVPacket *pkt) {
    if (!pkt)return;
    //阻塞
    while (!isExit) {
        mux.lock();
        if (packs.size() < maxList) {
            packs.push_back(pkt);
            mux.unlock();
            break;
        }
        mux.unlock();
        msleep(1);
    }
}
