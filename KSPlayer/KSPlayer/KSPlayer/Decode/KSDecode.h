#pragma once
#include <mutex>
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
extern void XFreePacket(AVPacket **pkt);
extern void XFreeFrame(AVFrame **frame);
class KSDecode {
public:
    bool isAudio = false;
    
    //当前解码到的pts
    long long pts = 0;
    
    //打开解码器,不管成功与否都释放para空间
    virtual bool Open(AVCodecParameters *para);
    
    //发送到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
    virtual bool Send(AVPacket *pkt);
    
    //获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
    //每次复制一份，由调用者释放 av_frame_free
    virtual AVFrame* Recv();
    
    virtual void Close();
    virtual void Clear();
    
    KSDecode();
    virtual ~KSDecode();
protected:
    AVCodecContext *codec = 0;
    std::mutex mux;
};

