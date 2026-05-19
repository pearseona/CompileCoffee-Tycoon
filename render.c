// SDL2 전체 렌더링
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void render_frame(SDL_Renderer* ren, TTF_Font* fnt_lg, TTF_Font* fnt_md, TTF_Font* fnt_sm, Game* g) {

	if (!ren || !g)
		return;

	// 배경화면 색상
	SDL_SetRenderDrawColor(ren, 30, 30, 35, 255); // 다크 그레이
	SDL_RenderClear(ren);

	// 상단 상태바
	SDL_Rect headerRect = { 20, 20, SCREEN_W - 40, 60 };
	SDL_SetRenderDrawColor(ren, 40, 60, 120, 255); // 블루
	SDL_RenderFillRect(ren, &headerRect);

	// 중앙 영역
	SDL_Rect mainRect = { 20, 100, SCREEN_W - 40, 400 };
	SDL_SetRenderDrawColor(ren, 50, 50, 55, 255); // 약간 더 밝은 회색
	SDL_RenderFillRect(ren, &mainRect);

	// 하단 로그 영역
	SDL_Rect logRect = { 20, 520, SCREEN_W - 40, 80 };
	SDL_SetRenderDrawColor(ren, 70, 40, 40, 255); // 딥 레드
	SDL_RenderFillRect(ren, &logRect);

	// 콘솔 로그
	static int frame_count = 0;
	if (frame_count++ % 60 == 0) {
		printf("Day: %d | Time Remaining: %d sec\n", g->day, g->day_ms / 1000);
	}

	SDL_RenderPresent(ren);
}