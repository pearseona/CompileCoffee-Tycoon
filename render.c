#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "render.h"

// 크기별 폰트
TTF_Font* g_fnt_lg = NULL;
TTF_Font* g_fnt_md = NULL;
TTF_Font* g_fnt_sm = NULL;

// 픽셀 그래픽 전역 변수
SDL_Texture* g_tex_customers[3] = { NULL, NULL, NULL };
SDL_Texture* g_tex_barista = NULL;
SDL_Texture* g_tex_station = NULL;
SDL_Texture* g_tex_background = NULL;

// 메뉴 미니 픽셀 이미지 텍스처
SDL_Texture* g_tex_menus[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

// 외부(render_ingame.c)에 선언된 개별 그리기 함수들
extern void draw_game_background(SDL_Renderer* ren);
extern void draw_customer_queue(SDL_Renderer* ren, Game* g);
extern void draw_barista_slots(SDL_Renderer* ren, Game* g);

/* 폰트 엔진 초기화 */
bool render_init_fonts() {
	g_fnt_lg = TTF_OpenFont("font.ttf", 26);
	g_fnt_md = TTF_OpenFont("font.ttf", 18);
	g_fnt_sm = TTF_OpenFont("font.ttf", 13);

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

/* 텍스처 로더: PNG를 먼저 시도하고 실패하면 BMP 로드 */
SDL_Texture* load_texture(SDL_Renderer* ren, const char* baseName) {
	char path[256]; // 경로 버퍼 확장
	SDL_Surface* surf = NULL;

	if (!baseName) return NULL;

	// 1. PNG 로드 시도 (sizeof 추가로 버퍼 오버런 완벽 방지)
	sprintf_s(path, sizeof(path), "%s.png", baseName);
	surf = IMG_Load(path);

	// 2. 실패 시 BMP 로드 시도
	if (!surf) {
		sprintf_s(path, sizeof(path), "%s.bmp", baseName);
		surf = SDL_LoadBMP(path);
	}

	if (!surf) {
		return NULL;
	}

	// BMP의 경우 흰색(255, 255, 255)을 투명 키 처리
	if (strstr(path, ".bmp") != NULL) {
		SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 255, 255));
	}

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	SDL_FreeSurface(surf);
	return tex;
}

/* 이미지 로드 파이프라인 */
bool render_init_images(SDL_Renderer* ren) {
	if (!ren) return false;

	// 배경화면
	g_tex_background = load_texture(ren, "background");
	if (!g_tex_background) {
		printf("[WARN] background 이미지 로드 실패! 기본 색상으로 채웁니다.\n");
	}

	// 손님들
	for (int i = 0; i < 3; i++) {
		char custName[64];
		sprintf_s(custName, sizeof(custName), "customer_%d", i);
		g_tex_customers[i] = load_texture(ren, custName);
		if (!g_tex_customers[i]) {
			printf("[WARN] %s 이미지 로드 실패!\n", custName);
		}
	}

	// 바리스타 및 작업대
	g_tex_barista = load_texture(ren, "barista");
	g_tex_station = load_texture(ren, "barista_station");

	// 음료 메뉴 아이콘들
	for (int i = 0; i < 6; i++) {
		char menuName[64];
		sprintf_s(menuName, sizeof(menuName), "menu_%d", i);
		g_tex_menus[i] = load_texture(ren, menuName);
	}

	return true;
}

/* 이미지 메모리 해제 */
void render_close_images() {
	if (g_tex_background) {
		SDL_DestroyTexture(g_tex_background);
		g_tex_background = NULL;
	}
	for (int i = 0; i < 3; i++) {
		if (g_tex_customers[i] != NULL) { // 확실하게 빈 값이 아닐 때만 해제
			SDL_DestroyTexture(g_tex_customers[i]);
			g_tex_customers[i] = NULL;
		}
	}
	if (g_tex_barista) {
		SDL_DestroyTexture(g_tex_barista);
		g_tex_barista = NULL;
	}
	if (g_tex_station) {
		SDL_DestroyTexture(g_tex_station);
		g_tex_station = NULL;
	}
	for (int i = 0; i < 6; i++) {
		if (g_tex_menus[i] != NULL) { // ⭐ 유령 포인터 오버런 완벽 방어!
			SDL_DestroyTexture(g_tex_menus[i]);
			g_tex_menus[i] = NULL;
		}
	}
}

/* 지정한 픽셀 좌표에 텍스트를 출력하는 공용 함수 */
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

/* 메인 렌더링 컨트롤러 */
void render_frame(SDL_Renderer* ren, Game* g) {
	if (!ren || !g) return;

	// 화면 기본 버퍼 클리어 (아늑한 브라운 다크 톤)
	SDL_SetRenderDrawColor(ren, 42, 33, 28, 255);
	SDL_RenderClear(ren);

	if (g->state == STATE_PLAYING || g->state == STATE_CLOSING) {
		// 1. 귀엽고 아늑한 배경 그리기
		draw_game_background(ren);

		// 2. 상단 상태 바 렌더링
		SDL_Rect headerBg = { 10, 10, SCREEN_W - 20, 50 };
		SDL_SetRenderDrawColor(ren, 26, 18, 14, 220);
		SDL_RenderFillRect(ren, &headerBg);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &headerBg);

		SDL_Color text_gold = { 253, 203, 110, 255 };
		SDL_Color text_white = { 255, 255, 255, 255 };

		// [왼쪽] 보유 금액 & 평판 (sizeof 추가 완료)
		char uiBuf[256];
		sprintf_s(uiBuf, sizeof(uiBuf), "🪙 %d원  [평판: %d/100]", g->balance, g->reputation);
		draw_text(ren, g_fnt_md, uiBuf, 25, 24, text_gold);

		// [중앙] 일차 정보 (sizeof 추가 완료)
		sprintf_s(uiBuf, sizeof(uiBuf), "DAY %02d", g->day);
		draw_text(ren, g_fnt_md, uiBuf, 310, 24, text_white);

		// 타이머 진행 바
		int bar_x = 390;
		int bar_y = 30;
		int bar_max_w = 340;
		int bar_h = 12;

		SDL_Rect timeBarBg = { bar_x, bar_y, bar_max_w, bar_h };
		SDL_SetRenderDrawColor(ren, 64, 48, 38, 255);
		SDL_RenderFillRect(ren, &timeBarBg);

		float time_ratio = (float)g->day_ms / 20000.0f;
		if (time_ratio < 0.0f) time_ratio = 0.0f;
		if (time_ratio > 1.0f) time_ratio = 1.0f;
		int time_curr_w = (int)(bar_max_w * time_ratio);

		SDL_Rect timeBarFill = { bar_x, bar_y, time_curr_w, bar_h };
		if (time_ratio > 0.5f) SDL_SetRenderDrawColor(ren, 120, 220, 100, 255);
		else if (time_ratio > 0.25f) SDL_SetRenderDrawColor(ren, 243, 156, 18, 255);
		else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
		SDL_RenderFillRect(ren, &timeBarFill);

		// [오른쪽] 디지털 시계
		int remaining_sec = g->day_ms / 1000;
		int display_m = remaining_sec / 60;
		int display_s = remaining_sec % 60;

		sprintf_s(uiBuf, sizeof(uiBuf), "%02d:%02d", display_m, display_s);
		draw_text(ren, g_fnt_md, uiBuf, 750, 24, text_white);

		// 데코 아이콘
		SDL_Rect decorationIcon = { SCREEN_W - 48, 20, 30, 30 };
		SDL_SetRenderDrawColor(ren, 225, 112, 85, 255);
		SDL_RenderFillRect(ren, &decorationIcon);
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderDrawRect(ren, &decorationIcon);

		// 3. 인게임 손님들 & 말풍선 렌더링
		draw_customer_queue(ren, g);

		// 4. 카운터 및 제조 기계/슬롯 렌더링
		draw_barista_slots(ren, g);

		// 5. 하단 텍스트 알림 피드
		SDL_Rect logRect = { 10, 545, SCREEN_W - 20, 65 };
		SDL_SetRenderDrawColor(ren, 33, 24, 18, 240);
		SDL_RenderFillRect(ren, &logRect);
		SDL_SetRenderDrawColor(ren, 75, 58, 46, 255);
		SDL_RenderDrawRect(ren, &logRect);

		SDL_Color log_color = { 236, 240, 241, 255 };
		SDL_Color tip_color = { 180, 180, 180, 255 };

		draw_text(ren, g_fnt_sm, "[조작팁] 마우스 클릭으로 슬롯 선택, 메뉴 터치, 손님 클릭 서빙! 또는 키보드 [1, 2, 3] 슬롯 선택, [Q~Y] 음료 제조, [A, S, D] 손님 서빙!", 25, 550, tip_color);

		if (g->log_count > 0) {
			draw_text(ren, g_fnt_md, g->log_lines[g->log_count - 1], 25, 577, log_color);
		}
		else {
			draw_text(ren, g_fnt_md, "아늑하고 귀여운 컴파일 카페가 영업 중입니다. 첫 손님을 환영해 주세요!", 25, 577, log_color);
		}
	}
	else if (g->state == STATE_UPGRADE) {
		SDL_Color gold = { 253, 203, 110, 255 };
		SDL_Color white = { 255, 255, 255, 255 };
		SDL_Color light_gray = { 200, 200, 200, 255 };
		char shopBuf[256];

		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 45, 33, 24, 255);
		SDL_RenderFillRect(ren, &shopBg);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &shopBg);

		sprintf_s(shopBuf, sizeof(shopBuf), "♥ 제 %d 일 차  영 업 준 비 단 계 ♥", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 80, gold);

		sprintf_s(shopBuf, sizeof(shopBuf), "현재 보유 잔액: %d원  |  보유 제조 슬롯: %d개  (속도 강화 Lv.%d)",
			g->balance, g->slot_count, g->upg[1].level);
		draw_text(ren, g_fnt_md, shopBuf, 70, 150, white);

		draw_text(ren, g_fnt_md, "매장을 업그레이드하고 다음 영업을 준비하세요!", 70, 210, gold);

		sprintf_s(shopBuf, sizeof(shopBuf), "[8]번 키 : 제조 슬롯 확장 (바리스타 추가 고용)  [비용: 3,000원] (%d/%d)",
			g->upg[0].level, g->upg[0].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, 90, 270, light_gray);

		sprintf_s(shopBuf, sizeof(shopBuf), "[9]번 키 : 고속 커피 머신 도입 (제조 속도 향상)  [비용: 25,000원] (%d/%d)",
			g->upg[1].level, g->upg[1].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, 90, 310, light_gray);

		draw_text(ren, g_fnt_md, "➔ 정비를 마쳤다면 [Enter]를 눌러 다음 날 영업을 시작합니다.", 70, SCREEN_H - 140, gold);
	}

	SDL_RenderPresent(ren);
}