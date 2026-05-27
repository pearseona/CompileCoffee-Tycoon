#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "./common.h"

void render_frame(SDL_Renderer* ren, TTF_Font* fnt_lg, TTF_Font* fnt_md, TTF_Font* fnt_sm, Game* g);

void draw_customer_queue(SDL_Renderer* ren, Game* g);
void draw_barista_slots(SDL_Renderer* ren, Game* g);