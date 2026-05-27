#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include "render.h"

//내부에서만 사용할 크기별 폰트
static TTF_Font* g_fnt_lg = NULL; // 대형 폰트 (32px)
static TTF_Font* g_fnt_md = NULL; // 중형 폰트 (20px)
static TTF_Font* g_fnt_sm = NULL; // 소형 폰트 (14px)

/* 폰트 엔진 준비 */
bool render_init_fonts() {
	g_fnt_lg = TTF_OpenFont("font.ttf", 32);
	g_fnt_md = TTF_OpenFont("font.ttf", 20);
	g_fnt_sm = TTF_OpenFont("font.ttf", 14);

	if (!g_fnt_lg || !g_fnt_md || !g_fnt_sm) {
		printf("[ERROR] font.ttf 로드 실패! 디스크 위치를 확인하세요.\n");
		return false;
	}
	return true;
}

/* 프로그램 종료 시 폰트 정리 */
void render_close_fonts() {
	if (g_fnt_lg) TTF_CloseFont(g_fnt_lg);
	if (g_fnt_md) TTF_CloseFont(g_fnt_md);
	if (g_fnt_sm) TTF_CloseFont(g_fnt_sm);
}

/* 메인 렌더링 컨트롤러 */
void render_frame(SDL_Renderer* ren, Game* g) {

	if (!ren || !g) return;

	// ========== 배경화면 및 메인 레이아웃 ==========

	/* 배경화면 */
	SDL_SetRenderDrawColor(ren, 30, 30, 35, 255); // 다크 그레이
	SDL_RenderClear(ren);


	/* 상단 상태바 */
	SDL_Rect headerRect = { 20, 20, SCREEN_W - 40, 45 };
	SDL_SetRenderDrawColor(ren, 40, 60, 120, 255); // 네이비 블루
	SDL_RenderFillRect(ren, &headerRect);


	/* 상단 시간 타이머 바 (영업 시간) */
	int max_timer_w = SCREEN_W - 40;
	int current_timer_w = (int)((float)g->day_ms / 90000.0f * max_timer_w);
	current_timer_w = clamp_i(current_timer_w, 0, max_timer_w);

	SDL_Rect timerBar = { 30, 70, current_timer_w, 10 };
	SDL_SetRenderDrawColor(ren, 230, 80, 80, 255); // 주황빛 빨간색
	SDL_RenderFillRect(ren, &timerBar);


	/* 중앙 게임 플레이 영역 (손님 대기 큐 및 제조 워크스테이션) */
	SDL_Rect mainRect = { 20, 95, SCREEN_W - 40, 405 };
	SDL_SetRenderDrawColor(ren, 50, 50, 55, 255); // 약간 더 밝은 회색
	SDL_RenderFillRect(ren, &mainRect);

	// 손님 큐 영역 호출
	draw_customer_queue(ren, g);

	// 바리스타 제조 영역 호출
	draw_barista_slots(ren, g);

	// ========== 하단 UI 및 잔액 ==========

	/*  잔액 영역 */
	int balance_w = clamp_i(g->balance / 500, 10, 200);
	SDL_Rect moneyRect = { SCREEN_W - 240, 25, balance_w, 35};
	SDL_SetRenderDrawColor(ren, 46, 139, 87, 255); // 초록색
	SDL_RenderFillRect(ren, &moneyRect);

	/* 하단 로그 영역 */
	SDL_Rect logRect = { 20, 520, SCREEN_W - 40, 80 };
	SDL_SetRenderDrawColor(ren, 70, 40, 40, 255); // 딥 레드
	SDL_RenderFillRect(ren, &logRect);
	
	/* 콘솔 로그 */
	if (g->state == STATE_PLAYING) {
		static int frame_count = 0;
		if (frame_count++ % 60 == 0) {
			printf("[HUD] Day: %d | 잔액: %d원 | 평판: %d | 남은시간: %d초\n",
				g->day, g->balance, g->reputation, g->day_ms / 1000);
		}
	}

	SDL_RenderPresent(ren);
}