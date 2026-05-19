// SDL2 전체 렌더링
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void render_frame(SDL_Renderer* ren, TTF_Font* fnt_lg, TTF_Font* fnt_md, TTF_Font* fnt_sm, Game* g) {

	if (!ren || !g)
		return;

	/* 배경화면 */
	SDL_SetRenderDrawColor(ren, 30, 30, 35, 255); // 다크 그레이
	SDL_RenderClear(ren);


	/* 상단 상태바 */
	SDL_Rect headerRect = { 20, 20, SCREEN_W - 40, 60 };
	SDL_SetRenderDrawColor(ren, 40, 60, 120, 255); // 블루
	SDL_RenderFillRect(ren, &headerRect);


	/* 실시간으로 남은 시간 타이머 */
	int max_bar_w = SCREEN_W - 60;
	int current_bar_w = (int)((float)g->day_ms / 90000.0f * max_bar_w);
	if (current_bar_w < 0) current_bar_w = 0;

	SDL_Rect timerBar = { 30, 65, current_bar_w, 10 };
	SDL_SetRenderDrawColor(ren, 230, 80, 80, 255); // 주황빛 빨간색
	SDL_RenderFillRect(ren, &timerBar);


	/* 중앙 게임 플레이 영역 */
	SDL_Rect mainRect = { 20, 100, SCREEN_W - 40, 400 };
	SDL_SetRenderDrawColor(ren, 50, 50, 55, 255); // 약간 더 밝은 회색
	SDL_RenderFillRect(ren, &mainRect);


	/*  잔고 영역 */
	int balance_w = clamp_i(g->balance / 200, 10, 300);
	SDL_Rect moneyRect = { 50, 130, balance_w, 40 };
	SDL_SetRenderDrawColor(ren, 46, 139, 87, 255); // 초록색
	SDL_RenderFillRect(ren, &moneyRect);


	/* 하단 로그 영역 */
	SDL_Rect logRect = { 20, 520, SCREEN_W - 40, 80 };
	SDL_SetRenderDrawColor(ren, 70, 40, 40, 255); // 딥 레드
	SDL_RenderFillRect(ren, &logRect);
	
	/* 콘솔 로그 */
	static int frame_count = 0;
	if (frame_count++ % 60 == 0) {
		printf("Day: %d | Time Remaining: %d sec\n", g->day, g->day_ms / 1000);
	}

	SDL_RenderPresent(ren);
}