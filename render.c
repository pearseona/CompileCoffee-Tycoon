#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "render.h"

TTF_Font* g_fnt_lg = NULL;
TTF_Font* g_fnt_md = NULL;
TTF_Font* g_fnt_sm = NULL;

SDL_Texture* g_tex_customers[3] = { NULL, NULL, NULL };
SDL_Texture* g_tex_barista = NULL;
SDL_Texture* g_tex_station = NULL;
SDL_Texture* g_tex_background = NULL;
SDL_Texture* g_tex_menus[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

extern void draw_game_background(SDL_Renderer* ren);
extern void draw_customer_queue(SDL_Renderer* ren, Game* g);
extern void draw_barista_slots(SDL_Renderer* ren, Game* g);

bool render_init_fonts() {
	g_fnt_lg = TTF_OpenFont("font.ttf", 26);
	g_fnt_md = TTF_OpenFont("font.ttf", 18);
	g_fnt_sm = TTF_OpenFont("font.ttf", 13);
	if (!g_fnt_lg || !g_fnt_md || !g_fnt_sm) return false;
	return true;
}

void render_close_fonts() {
	if (g_fnt_lg) TTF_CloseFont(g_fnt_lg);
	if (g_fnt_md) TTF_CloseFont(g_fnt_md);
	if (g_fnt_sm) TTF_CloseFont(g_fnt_sm);
}

SDL_Texture* load_texture(SDL_Renderer* ren, const char* baseName) {
	char path[256];
	SDL_Surface* surf = NULL;
	if (!baseName) return NULL;
	sprintf_s(path, sizeof(path), "%s.png", baseName);
	surf = IMG_Load(path);
	if (!surf) {
		sprintf_s(path, sizeof(path), "%s.bmp", baseName);
		surf = SDL_LoadBMP(path);
	}
	if (!surf) return NULL;
	if (strstr(path, ".bmp") != NULL) {
		SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 255, 255));
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	SDL_FreeSurface(surf);
	return tex;
}

bool render_init_images(SDL_Renderer* ren) {
	if (!ren) return false;
	g_tex_background = load_texture(ren, "background");
	for (int i = 0; i < 3; i++) {
		char custName[64];
		sprintf_s(custName, sizeof(custName), "customer_%d", i);
		g_tex_customers[i] = load_texture(ren, custName);
	}
	g_tex_barista = load_texture(ren, "barista");
	g_tex_station = load_texture(ren, "barista_station");
	for (int i = 0; i < 6; i++) {
		char menuName[64];
		sprintf_s(menuName, sizeof(menuName), "menu_%d", i);
		g_tex_menus[i] = load_texture(ren, menuName);
	}
	return true;
}

void render_close_images() {
	if (g_tex_background) SDL_DestroyTexture(g_tex_background);
	for (int i = 0; i < 3; i++) {
		if (g_tex_customers[i]) SDL_DestroyTexture(g_tex_customers[i]);
	}
	if (g_tex_barista) SDL_DestroyTexture(g_tex_barista);
	if (g_tex_station) SDL_DestroyTexture(g_tex_station);
	for (int i = 0; i < 6; i++) {
		if (g_tex_menus[i]) SDL_DestroyTexture(g_tex_menus[i]);
	}
}

void draw_text(SDL_Renderer* ren, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
	if (!ren || !font || !text || text[0] == '\0') return;
	SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
	if (!surf) return;
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	if (!tex) { SDL_FreeSurface(surf); return; }
	SDL_Rect dstRect = { x, y, surf->w, surf->h };
	SDL_RenderCopy(ren, tex, NULL, &dstRect);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
}

void render_frame(SDL_Renderer* ren, Game* g) {
	if (!ren || !g) return;
	SDL_SetRenderDrawColor(ren, 42, 33, 28, 255);
	SDL_RenderClear(ren);

	if (g->state == STATE_PLAYING || g->state == STATE_CLOSING) {
		draw_game_background(ren);
		SDL_Rect headerBg = { 10, 10, SCREEN_W - 20, 50 };
		SDL_SetRenderDrawColor(ren, 26, 18, 14, 220);
		SDL_RenderFillRect(ren, &headerBg);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &headerBg);

		SDL_Color text_gold = { 253, 203, 110, 255 };
		SDL_Color text_white = { 255, 255, 255, 255 };

		char uiBuf[256];
		sprintf_s(uiBuf, sizeof(uiBuf), "🪙 %d원  [평판: %d/100]", g->balance, g->reputation);
		draw_text(ren, g_fnt_md, uiBuf, 25, 24, text_gold);

		sprintf_s(uiBuf, sizeof(uiBuf), "DAY %02d", g->day);
		draw_text(ren, g_fnt_md, uiBuf, 310, 24, text_white);

		int bar_x = 390; int bar_y = 30; int bar_max_w = 340; int bar_h = 12;
		SDL_Rect timeBarBg = { bar_x, bar_y, bar_max_w, bar_h };
		SDL_SetRenderDrawColor(ren, 64, 48, 38, 255);
		SDL_RenderFillRect(ren, &timeBarBg);

		float time_ratio = (float)g->day_ms / 90000.0f;
		if (time_ratio < 0.0f) time_ratio = 0.0f;
		if (time_ratio > 1.0f) time_ratio = 1.0f;
		int time_curr_w = (int)(bar_max_w * time_ratio);

		SDL_Rect timeBarFill = { bar_x, bar_y, time_curr_w, bar_h };
		if (time_ratio > 0.5f) SDL_SetRenderDrawColor(ren, 120, 220, 100, 255);
		else if (time_ratio > 0.25f) SDL_SetRenderDrawColor(ren, 243, 156, 18, 255);
		else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
		SDL_RenderFillRect(ren, &timeBarFill);

		int remaining_sec = g->day_ms / 1000;
		int display_m = remaining_sec / 60;
		int display_s = remaining_sec % 60;
		sprintf_s(uiBuf, sizeof(uiBuf), "%02d:%02d", display_m, display_s);
		draw_text(ren, g_fnt_md, uiBuf, 750, 24, text_white);

		SDL_Rect decorationIcon = { SCREEN_W - 48, 20, 30, 30 };
		SDL_SetRenderDrawColor(ren, 225, 112, 85, 255);
		SDL_RenderFillRect(ren, &decorationIcon);

		draw_customer_queue(ren, g);
		draw_barista_slots(ren, g);

		SDL_Rect logRect = { 10, 545, SCREEN_W - 20, 65 };
		SDL_SetRenderDrawColor(ren, 33, 24, 18, 240);
		SDL_RenderFillRect(ren, &logRect);
		SDL_SetRenderDrawColor(ren, 75, 58, 46, 255);
		SDL_RenderDrawRect(ren, &logRect);

		SDL_Color log_color = { 236, 240, 241, 255 };
		SDL_Color tip_color = { 180, 180, 180, 255 };
		draw_text(ren, g_fnt_sm, "[조작팁] 마우스 클릭 슬롯 지정, 메뉴 선택 제조, 고객 클릭 서빙! [Enter] 상점 전환", 25, 550, tip_color);

		if (g->log_count > 0) {
			draw_text(ren, g_fnt_md, g->log_lines[(g->log_count - 1) % MAX_LOG_LINES], 25, 577, log_color);
		}
		else {
			draw_text(ren, g_fnt_md, "어서오세요! 컴파일 커피가 정상 영업을 개시했습니다.", 25, 577, log_color);
		}
	}
	else if (g->state == STATE_UPGRADE) {
		SDL_Color gold = { 253, 203, 110, 255 };
		SDL_Color white = { 255, 255, 255, 255 };
		SDL_Color light_gray = { 200, 200, 200, 255 };
		SDL_Color card_color = { 58, 43, 33, 255 };
		char shopBuf[256];

		// 🏰 메인 상점 원목 대형 보드판 배경
		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 43, 30, 22, 255);
		SDL_RenderFillRect(ren, &shopBg);
		SDL_SetRenderDrawColor(ren, 253, 203, 110, 255);
		SDL_RenderDrawRect(ren, &shopBg);

		// 헤더 자산 바 정보
		sprintf_s(shopBuf, sizeof(shopBuf), "♥ 제 %d 일 차  영 업 준 비 단 계 (상점) ♥", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 70, gold);

		sprintf_s(shopBuf, sizeof(shopBuf), "현재 보유 잔액: 🪙 %d원  |  보유 제조 슬롯: %d개  (속도 강화 Lv.%d)", g->balance, g->slot_count, g->upg[1].level);
		draw_text(ren, g_fnt_md, shopBuf, 70, 125, white);

		// 📦 [아이템 카드 1: 제조 슬롯 확장 플레이트]
		SDL_Rect card1 = { 70, 180, SCREEN_W - 140, 90 };
		SDL_SetRenderDrawColor(ren, card_color.r, card_color.g, card_color.b, 255);
		SDL_RenderFillRect(ren, &card1);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &card1);

		draw_text(ren, g_fnt_md, "🔨 [8]번 키 : 제조 슬롯 확장 (바리스타 추가 고용)", 90, 195, gold);
		sprintf_s(shopBuf, sizeof(shopBuf), "동시에 음료를 제조할 수 있는 슬롯을 추가합니다. [비용: 3,000원] (현재: %d / 최대: %d)", g->slot_count, g->upg[0].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, 90, 232, light_gray);

		// 📦 [아이템 카드 2: 머신 고속화 플레이트]
		SDL_Rect card2 = { 70, 290, SCREEN_W - 140, 90 };
		SDL_SetRenderDrawColor(ren, card_color.r, card_color.g, card_color.b, 255);
		SDL_RenderFillRect(ren, &card2);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &card2);

		draw_text(ren, g_fnt_md, "⚡ [9]번 키 : 고속 커피 머신 도입 (제조 속도 향상)", 90, 305, gold);
		sprintf_s(shopBuf, sizeof(shopBuf), "음료 제조 속도가 단계별로 15%%씩 빨라집니다. [비용: 25,000원] (현재 레벨: Lv.%d / 최대: %d)", g->upg[1].level, g->upg[1].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, 90, 342, light_gray);

		// 🧾 하단 가이드 바
		SDL_Rect bottomGuide = { 70, 410, SCREEN_W - 140, 50 };
		SDL_SetRenderDrawColor(ren, 26, 18, 14, 255);
		SDL_RenderFillRect(ren, &bottomGuide);
		draw_text(ren, g_fnt_md, "➔ 정비를 마쳤다면 [Enter]를 눌러 다음 영업을 시작합니다.", 90, 423, gold);

		// 실시간 로그 알림 피드백 연동
		if (g->log_count > 0) {
			draw_text(ren, g_fnt_sm, g->log_lines[(g->log_count - 1) % MAX_LOG_LINES], 70, 485, white);
		}
	}
	SDL_RenderPresent(ren);
}