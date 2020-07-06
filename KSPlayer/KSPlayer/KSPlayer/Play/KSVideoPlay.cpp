//
//  KSVideoPlay.cpp
//  KSPlayer
//
//  Created by saeipi on 2020/7/6.
//  Copyright © 2020 saeipi. All rights reserved.
//

#include "KSVideoPlay.h"
extern "C" {
#include "libavformat/avformat.h"
}

void KSVideoPlay::CreateSDL() {
    ctx_width = 640;
    ctx_height = 360;
    sdl_window = SDL_CreateWindow("KSPlayer",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  ctx_width,
                                  ctx_height,
                                  SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    
    if (!sdl_window) {
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());
        return;
    }
    
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    
    sdl_texture  = SDL_CreateTexture(sdl_renderer,
                                     SDL_PIXELFORMAT_IYUV,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     ctx_width,
                                     ctx_height);
    
    sdl_rect.x   = 0;
    sdl_rect.y   = 0;
    sdl_rect.w   = ctx_width;
    sdl_rect.h   = ctx_height;
}

void KSVideoPlay::Init(int width, int height) {
    
}

void KSVideoPlay::Repaint(AVFrame *frame) {
    if (frame->width != ctx_width || frame->height != ctx_height) {
        mux.lock();
        ctx_width = frame->width;
        ctx_height = frame->height;
        mux.unlock();
        return;
    }
    
    sdl_rect.x   = 0;
    sdl_rect.y   = 0;
    sdl_rect.w   = frame->width;
    sdl_rect.h   = frame->height;
    SDL_UpdateTexture(sdl_texture,
                      NULL,
                      frame->data[0],
                      frame->linesize[0]);
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
}
