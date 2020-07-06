//
//  ViewController.m
//  KSPlayer
//
//  Created by saeipi on 2020/7/6.
//  Copyright © 2020 saeipi. All rights reserved.
//

#import "ViewController.h"
#include "KSDemuxThread.h"
#include "KSVideoPlay.h"

@interface ViewController() {
    KSDemuxThread *demuxThread;
}

@end
@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    demuxThread = new KSDemuxThread();
    KSVideoPlay *videoPlay = new KSVideoPlay();
    videoPlay->CreateSDL();
    
    char *url = "/Users/saeipi/Downloads/File/SOPSandwich.mp4";
    demuxThread->Open(url,videoPlay);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        self->demuxThread->Start();
    });
    

    // Do any additional setup after loading the view.
    
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

@end
