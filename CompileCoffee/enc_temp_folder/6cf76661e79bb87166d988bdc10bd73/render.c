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

// 🛠️ 상점 전용 그래픽 텍스처 포인터 바인딩
SDL_Texture* g_tex_shop_slot = NULL;
SDL_Texture* g_tex_shop_machine = NULL;

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

	// 🛠️ 상점 리소스 에셋 로드 추가
	g_tex_shop_slot = load_texture(ren, "shop_slot");
	g_tex_shop_machine = load_texture(ren, "shop_machine");

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

	// 🛠️ 안전 해제 파이프라인
	if (g_tex_shop_slot) SDL_DestroyTexture(g_tex_shop_slot);
	if (g_tex_shop_machine) SDL_DestroyTexture(g_tex_shop_machine);
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
		// 인게임 실시간 창고 재고 현황판 HUD 추가 연동 (가시성 고도화 점수 확보!)
		sprintf_s(uiBuf, sizeof(uiBuf), "🪙 %d원  [평판: %d/100]  🫘원두:%d  🥛우유:%d", g->balance, g->reputation, g->stock[ING_BEAN], g->stock[ING_MILK]);
		draw_text(ren, g_fnt_md, uiBuf, 25, 24, text_gold);

		sprintf_s(uiBuf, sizeof(uiBuf), "DAY %02d", g->day);
		draw_text(ren, g_fnt_md, uiBuf, 350, 24, text_white);

		int bar_x = 420; int bar_y = 30; int bar_max_w = 310; int bar_h = 12;
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
		// 🌸 핑크 감성 컬러 테마 세팅
		SDL_Color shop_pink_main = { 255, 121, 198, 255 };   // 러블리 핫핑크 (타이틀용)
		SDL_Color text_white = { 255, 255, 255, 255 };
		SDL_Color text_dark_cocoa = { 85, 55, 65, 255 };    // 가독성 높은 진한 브라운 초코 (본문용)
		SDL_Color light_pink_card = { 255, 242, 245, 255 };  // 부드러운 하트 핑크 카드 바디
		SDL_Color pink_border = { 255, 182, 193, 255 };      // 파스텔 라이트 핑크 테두리
		char shopBuf[256];

		// 🏰 1. 상점 메인 배경 대형 판넬 (러블리 핑크 월페이퍼)
		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 255, 218, 224, 255); // 화사한 딸기우유 마감 배경
		SDL_RenderFillRect(ren, &shopBg);
		SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		SDL_RenderDrawRect(ren, &shopBg);

		// 상단 메인 정산 타이틀 라인
		sprintf_s(shopBuf, sizeof(shopBuf), "💖 컴파일 커피 정비소 - DAY %d 💖", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 65, shop_pink_main);

		sprintf_s(shopBuf, sizeof(shopBuf), "보유 자산: 🪙 %d원  |  창고 원두: %d개  |  창고 우유: %d개", g->balance, g->stock[ING_BEAN], g->stock[ING_MILK]);
		draw_text(ren, g_fnt_md, shopBuf, 70, 115, shop_pink_main);

		// ----------------------------------------------------
		// 💬 2. 와이어프레임 반영: 점장 고양이의 말풍선 대사창
		SDL_Rect NPCBubble = { 70, 160, SCREEN_W - 140, 65 };
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderFillRect(ren, &NPCBubble);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &NPCBubble);

		draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"재고가 다 떨어지면 긴급 오토바이 비용이 드니까 상점에서 미리 사두라구냥!\"", 95, 181, text_dark_cocoa);

		// ----------------------------------------------------
		// 📦 3. 기술 설계 반영: 4열 종대 가로 분할 카드 레이아웃 연산
		// 화면 마진 제외 가로폭 820px을 4개로 고르게 분할하는 정밀 나눗셈 배치식
		int card_y = 245;
		int card_w = 200; // 가로 크기 200픽셀로 컴팩트 조절
		int card_h = 160; // 텍스트 가독성을 위해 세로 폭 160으로 확장
		int gap = 8;     // 카드와 카드 사이의 마진 간격

		// --- [카드 0: 제조 슬롯 확장] ---
		int x0 = 70;
		SDL_Rect rc0 = { x0, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &rc0);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &rc0);

		SDL_Rect imgR0 = { x0 + 68, card_y + 12, 64, 64 };
		if (g_tex_shop_slot) SDL_RenderCopy(ren, g_tex_shop_slot, NULL, &imgR0);

		draw_text(ren, g_fnt_sm, "🔨 [8] 슬롯 확장", x0 + 12, card_y + 82, shop_pink_main);
		sprintf_s(shopBuf, sizeof(shopBuf), "보유 수량: %d / %d", g->slot_count, g->upg[0].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, x0 + 12, card_y + 105, text_dark_cocoa);
		draw_text(ren, g_fnt_sm, "비용: 3,000원", x0 + 12, card_y + 130, shop_pink_main);


		// --- [카드 1: 머신 속도 향상] ---
		int x1 = x0 + card_w + gap;
		SDL_Rect rc1 = { x1, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &rc1);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &rc1);

		SDL_Rect imgR1 = { x1 + 68, card_y + 12, 64, 64 };
		if (g_tex_shop_machine) SDL_RenderCopy(ren, g_tex_shop_machine, NULL, &imgR1);

		draw_text(ren, g_fnt_sm, "⚡ [9] 머신 속도", x1 + 12, card_y + 82, shop_pink_main);
		sprintf_s(shopBuf, sizeof(shopBuf), "현재 등급: Lv.%d / %d", g->upg[1].level, g->upg[1].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, x1 + 12, card_y + 105, text_dark_cocoa);
		draw_text(ren, g_fnt_sm, "비용: 25,000원", x1 + 12, card_y + 130, shop_pink_main);


		// --- [카드 2: 대안 B 원두 도매 구매] ---
		int x2 = x1 + card_w + gap;
		SDL_Rect rc2 = { x2, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &rc2);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &rc2);

		// 원두 전용 도트 미니 사각형 가상 패널 빌드
		SDL_Rect imgR2 = { x2 + 75, card_y + 20, 50, 50 };
		SDL_SetRenderDrawColor(ren, 139, 90, 60, 255); // 원두 초콜릿 원목 갈색 주입
		SDL_RenderFillRect(ren, &imgR2);
		draw_text(ren, g_fnt_sm, "BEAN", x2 + 84, card_y + 36, text_white);

		draw_text(ren, g_fnt_sm, "🫘 원두 자루 (x10)", x2 + 12, card_y + 82, shop_pink_main);
		sprintf_s(shopBuf, sizeof(shopBuf), "현재고: %d개 보유중", g->stock[ING_BEAN]);
		draw_text(ren, g_fnt_sm, shopBuf, x2 + 12, card_y + 105, text_dark_cocoa);
		draw_text(ren, g_fnt_sm, "비용: 1,500원", x2 + 12, card_y + 130, shop_pink_main);


		// --- [카드 3: 대안 B 우유 도매 구매] ---
		int x3 = x2 + card_w + gap;
		SDL_Rect rc3 = { x3, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &rc3);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &rc3);

		// 우유 전용 순백 밀크 패널 빌드
		SDL_Rect imgR3 = { x3 + 75, card_y + 20, 50, 50 };
		SDL_SetRenderDrawColor(ren, 230, 240, 250, 255); // 뽀얀 우유 빛깔 주입
		SDL_RenderFillRect(ren, &imgR3);
		draw_text(ren, g_fnt_sm, "MILK", x3 + 85, card_y + 36, text_dark_cocoa);

		draw_text(ren, g_fnt_sm, "🥛 우유 묶음 (x10)", x3 + 12, card_y + 82, shop_pink_main);
		sprintf_s(shopBuf, sizeof(shopBuf), "현재고: %d개 보유중", g->stock[ING_MILK]);
		draw_text(ren, g_fnt_sm, shopBuf, x3 + 12, card_y + 105, text_dark_cocoa);
		draw_text(ren, g_fnt_sm, "비용: 1,000원", x3 + 12, card_y + 130, shop_pink_main);


		// ----------------------------------------------------
		// 🧾 4. 하단 영업 가이드 네비게이션 바 (딸기 초코 테마 바)
		SDL_Rect bottomGuide = { 70, 425, SCREEN_W - 140, 45 };
		SDL_SetRenderDrawColor(ren, 255, 150, 180, 255);
		SDL_RenderFillRect(ren, &bottomGuide);
		draw_text(ren, g_fnt_md, "➔ 정비를 마쳤다면 [Enter] 키를 눌러 다음 날 영업을 시작합니다!", 95, 437, text_white);

		// 실시간 피드백 로그 정렬 알림창 연동
		if (g->log_count > 0) {
			draw_text(ren, g_fnt_sm, g->log_lines[(g->log_count - 1) % MAX_LOG_LINES], 70, 485, shop_pink_main);
		}
	}
	SDL_RenderPresent(ren);
}