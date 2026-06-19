#pragma once
#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "common.h"

// 폰트 및 이미지 자원 관리 초기화/해제 파이프라인
bool render_init_fonts(void);
void render_close_fonts(void);
bool render_init_images(SDL_Renderer* ren);
void render_close_images(void);

void render_frame(SDL_Renderer* ren, Game* g);

void draw_customer_queue(SDL_Renderer* ren, Game* g);
void draw_barista_slots(SDL_Renderer* ren, Game* g);
void draw_text(SDL_Renderer* ren, TTF_Font* font, const char* text, int x, int y, SDL_Color color);

#endif