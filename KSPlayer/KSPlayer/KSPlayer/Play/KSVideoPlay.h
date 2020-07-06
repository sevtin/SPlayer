//
//  KSVideoPlay.hpp
//  KSPlayer
//
//  Created by saeipi on 2020/7/6.
//  Copyright © 2020 saeipi. All rights reserved.
//

#include "KSProtocol.h"
#include <mutex>
extern "C" {
#include "SDL2/SDL.h"
}

class KSVideoPlay: public KSProtocol {
public:
    virtual void Init(int width, int height);
    virtual void Repaint(AVFrame *frame);
    void CreateSDL();
    
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *sdl_texture;
    SDL_Rect sdl_rect;
    int ctx_width;
    int ctx_height;
};
