#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include "render.h"

// 크기별 폰트
static TTF_Font* g_fnt_lg = NULL; // 대형 폰트 
static TTF_Font* g_fnt_md = NULL; // 중형 폰트 
static TTF_Font* g_fnt_sm = NULL; // 소형 폰트 )

// 픽셀 그래픽
SDL_Texture* g_tex_customers[3] = { NULL, NULL, NULL };
SDL_Texture* g_tex_barista = NULL;
SDL_Texture* g_tex_station = NULL;

/* 폰트 엔진 초기화 */
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

/* 픽셀 이미지 로드 */
bool render_init_images(SDL_Renderer* ren) {
	if (!ren) return false;

	// 손님 3명 로드 (직장인, 미식가, 학생)
	char fileName[64];
	for (int i = 0; i < 3; i++) {
		sprintf(fileName, "customer_%d.bmp", i);
		SDL_Surface* surf = SDL_LoadBMP(fileName);

		if (!surf) {
			printf("[ERROR] %s 로드 실패! 파일 위치를 확인하세요.\n", fileName);
			return false;
		}

		SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 255, 255));
		g_tex_customers[i] = SDL_CreateTextureFromSurface(ren, surf);
		SDL_FreeSurface(surf);
	}
	 
	// 바리스타 로드
	SDL_Surface* surf_barista = SDL_LoadBMP("barista.bmp");
	if (!surf_barista) {
		printf("[ERROR] barista.bmp 로드 실패! 파일 위치를 확인하세요.\n");
		return false;
	}

	SDL_SetColorKey(surf_barista, SDL_TRUE, SDL_MapRGB(surf_barista->format, 255, 255, 255));
	g_tex_barista = SDL_CreateTextureFromSurface(ren, surf_barista);
	SDL_FreeSurface(surf_barista);

	// 작업대 로드
	SDL_Surface* surf_stat = SDL_LoadBMP("barista_station.bmp");
	if (surf_stat) {
		g_tex_station = SDL_CreateTextureFromSurface(ren, surf_stat);
		SDL_FreeSurface(surf_stat);
	}

	return true;
}

/* 프로그램 종료 시 그래픽 메모리 해제 */
void render_close_images() {
	
	// 손님 3명 해제
	for (int i = 0; i < 3; i++) {
		if (g_tex_customers[i]) {
			SDL_DestroyTexture(g_tex_customers[i]);
			g_tex_customers[i] = NULL;
		}
	}

	// 바리스타 해제
	if (g_tex_barista) {
		SDL_DestroyTexture(g_tex_barista);
		g_tex_barista = NULL;
	}

	// 작업대 해제
	if (g_tex_station) {
		SDL_DestroyTexture(g_tex_station);
		g_tex_station = NULL;
	}
}

/* 메인 렌더링 컨트롤러 */
void render_frame(SDL_Renderer* ren, Game* g) {
	if (!ren || !g) return;

	/* ==== 공통 배경화면 초기화 ===== */
	SDL_SetRenderDrawColor(ren, 30, 30, 35, 255); 
	SDL_RenderClear(ren);
	
	if (g->state == STATE_PLAYING || g->state == STATE_CLOSING) {

		// 상단 상태바
		SDL_Rect headerRect = { 20, 20, SCREEN_W - 40, 45 };
		SDL_SetRenderDrawColor(ren, 40, 60, 120, 255); 
		SDL_RenderFillRect(ren, &headerRect);

		// 상단 시간 타이머 바
		int max_timer_w = SCREEN_W - 40;
		int current_timer_w = (int)((float)g->day_ms / 20000.0f * max_timer_w); // 20초
		current_timer_w = clamp_i(current_timer_w, 0, max_timer_w);

		SDL_Rect timerBar = { 30, 70, current_timer_w, 10 };
		SDL_SetRenderDrawColor(ren, 230, 80, 80, 255); 
		SDL_RenderFillRect(ren, &timerBar);

		// 중앙 게임 플레이 영역
		SDL_Rect mainRect = { 20, 95, SCREEN_W - 40, 405 };
		SDL_SetRenderDrawColor(ren, 50, 50, 55, 255); 
		SDL_RenderFillRect(ren, &mainRect);

		// 모듈별 렌더링 호출 
		draw_customer_queue(ren, g);
		draw_barista_slots(ren, g);

		// 하단 텍스트 로그 영역 배경
		SDL_Rect logRect = { 20, 520, SCREEN_W - 40, 80 };
		SDL_SetRenderDrawColor(ren, 70, 40, 40, 255); 
		SDL_RenderFillRect(ren, &logRect);

		/* ========== 텍스트 출력 ========== */
		SDL_Color white = { 255, 255, 255, 255 };
		char textBuf[128];

		// 영업 일차 및 평판 출력
		sprintf(textBuf, "Day %d  |  평판: %d / 100", g->day, g->reputation);
		draw_text(ren, g_fnt_md, textBuf, 40, 32, white);

		// 남은 영업 시간 실시간 타이머 초단위 출력
		sprintf(textBuf, "남은 시간: %d초", g->day_ms / 1000);
		draw_text(ren, g_fnt_md, textBuf, SCREEN_W - 180, 32, white);

		// 잔액 바 우측 정렬 텍스트 출력
		sprintf(textBuf, "보유 잔액: %d원", g->balance);
		draw_text(ren, g_fnt_md, textBuf, SCREEN_W - 420, 32, white);

		// 실시간 3초 입장 로그 연동
		SDL_Color light_gray = { 200, 200, 200, 255 };
		SDL_Color dark_gray = { 130, 130, 135, 255 };

		bool show_entrance_log = false;
		bool has_any_customer = false;

		for (int i = 0; i < 8; i++) {
			if (g->queue[i].active == 1) {
				has_any_customer = true;

				int patience_elapsed = g->queue[i].patience_max - g->queue[i].patience_ms;
				if (patience_elapsed >= 0 && patience_elapsed <= 3000) {
					show_entrance_log = true;
					break;
				}
			}
		}

		if (show_entrance_log) {
			draw_text(ren, g_fnt_sm, "손님이 입장했습니다. 주문을 확인하고 커피를 제조하세요.", 40, 545, light_gray);
		}
		else if (has_any_customer) {
			draw_text(ren, g_fnt_sm, "손님들이 주문을 기다리고 있습니다. 작업대 슬롯을 확인하세요.", 40, 545, light_gray);
		}
		else {
			draw_text(ren, g_fnt_sm, "현재 대기 중인 손님이 없습니다. 매장이 한적합니다.", 40, 545, dark_gray);
		}

	}
	else if (g->state == STATE_UPGRADE) {
		// 상점 정비 단계
		SDL_Color gold = { 241, 196, 15, 255 };
		SDL_Color white = { 255, 255, 255, 255 };
		SDL_Color light_gray = { 180, 180, 180, 255 };
		char shopBuf[128];

		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 45, 35, 30, 255);
		SDL_RenderFillRect(ren, &shopBg);

		sprintf(shopBuf, "[ 제 %d 일 차  영 업 준 비 단 계 ]", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 80, gold);

		sprintf(shopBuf, "현재 보유 잔액: %d원  |  보유 제조 슬롯: %d개", g->balance, g->slot_count);
		draw_text(ren, g_fnt_md, shopBuf, 70, 150, white);

		draw_text(ren, g_fnt_md, "매장을 업그레이드하고 다음 영업을 준비하세요!", 70, 230, white);
		draw_text(ren, g_fnt_sm, "[8]번 키 : 바리스타 추가 고용 (제조 슬롯 확장)  [비용: 3,000원]", 90, 280, light_gray);
		draw_text(ren, g_fnt_sm, "[9]번 키 : 고속 커피머신 도입 (제조 속도 향상)  [비용: 25,000원]", 90, 310, light_gray);

		draw_text(ren, g_fnt_md, "➔ 정비를 마쳤다면 [Enter]를 눌러 다음 날 영업을 개시합니다.", 70, SCREEN_H - 140, gold);
	}

	if (g->state == STATE_PLAYING) {
		static int frame_count = 0;
		if (frame_count++ % 60 == 0) {
			printf("[HUD] Day: %d | 잔액: %d원 | 평판: %d | 남은시간: %d초\n",
				g->day, g->balance, g->reputation, g->day_ms / 1000);
		}
	}

	SDL_RenderPresent(ren);
}

/* 지정한 픽셀 좌표에 텍스트를 출력하는 함수 본체 */
void draw_text(SDL_Renderer* ren, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
	if (!ren || !font || !text || text[0] == '\0') return;

	SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
	if (!surf) return;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	if (!tex) {
		SDL_FreeSurface(surf);
		return;
	}

	SDL_Rect dstRect = { x, y, surf->w, surf->h };
	SDL_RenderCopy(ren, tex, NULL, &dstRect);

	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
}