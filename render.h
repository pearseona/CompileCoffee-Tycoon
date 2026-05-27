#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "./common.h"

// 폰트 시스템 초기화 및 해제 함수
bool render_init_fonts();
void render_close_fonts();

// main에서 호출할 렌더링 함수
void render_frame(SDL_Renderer* ren, Game* g);

// 서브 그리기 모듈
void draw_customer_queue(SDL_Renderer* ren, Game* g);
void draw_barista_slots(SDL_Renderer* ren, Game* g);