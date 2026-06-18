#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "./common.h"

// 폰트 및 이미지 자원 관리
bool render_init_fonts();
void render_close_fonts();
bool render_init_images(SDL_Renderer* ren);
void render_close_images();

void render_frame(SDL_Renderer* ren, Game* g);
void draw_text(SDL_Renderer* ren, TTF_Font* font, const char* text, int x, int y, SDL_Color color);
void draw_customer_queue(SDL_Renderer* ren, Game* g);
void draw_barista_slots(SDL_Renderer* ren, Game* g);