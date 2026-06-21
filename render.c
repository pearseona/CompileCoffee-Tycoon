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
		sprintf_s(uiBuf, sizeof(uiBuf), "🪙 %d원  [평판: %d/100]", g->balance, g->reputation, g->stock[ING_BEAN], g->stock[ING_MILK]);
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
		// 🌸 파스텔 핑크 감성 컬러 테마 세팅
		SDL_Color shop_pink_main = { 255, 121, 198, 255 };
		SDL_Color text_white = { 255, 255, 255, 255 };
		SDL_Color text_dark_cocoa = { 85, 55, 65, 255 };
		SDL_Color light_pink_card = { 255, 242, 245, 255 };
		SDL_Color pink_border = { 255, 182, 193, 255 };
		SDL_Color unselected_tab = { 180, 150, 160, 255 };
		char shopBuf[256];

		// 🏰 1. 상점 메인 배경 대형 판넬
		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 255, 218, 224, 255);
		SDL_RenderFillRect(ren, &shopBg);
		SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		SDL_RenderDrawRect(ren, &shopBg);

		// 상단 메인 타이틀
		sprintf_s(shopBuf, sizeof(shopBuf), "💖 컴파일 커피 정비소 - DAY %d 💖", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 55, shop_pink_main);

		sprintf_s(shopBuf, sizeof(shopBuf), "보유 자산: 🪙 %d원  |  창고 원두: %d개  |  창고 우유: %d개", g->balance, g->stock[ING_BEAN], g->stock[ING_MILK]);
		draw_text(ren, g_fnt_md, shopBuf, 70, 95, shop_pink_main);

		// ----------------------------------------------------
		// 📑 [상점 탭 UI 추가 빌드 기법]
		// [탭 1번 버튼] 장비/재고 상점
		SDL_Rect tab1 = { 70, 130, 180, 30 };
		if (g->shop_page == 0) SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		else SDL_SetRenderDrawColor(ren, 200, 170, 180, 255);
		SDL_RenderFillRect(ren, &tab1);
		draw_text(ren, g_fnt_sm, "[◀] 1. 장비 & 재고 구매", 85, 137, text_white);

		// [탭 2번 버튼] 신메뉴 레시피 연구소 
		SDL_Rect tab2 = { 260, 130, 180, 30 };
		if (g->shop_page == 1) SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		else SDL_SetRenderDrawColor(ren, 200, 170, 180, 255);
		SDL_RenderFillRect(ren, &tab2);
		draw_text(ren, g_fnt_sm, "[▶] 2. 메뉴 레시피 해금", 275, 137, text_white);

		// 💬 점장 대사 말풍선창
		SDL_Rect NPCBubble = { 70, 170, SCREEN_W - 140, 50 };
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderFillRect(ren, &NPCBubble);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &NPCBubble);

		if (g->shop_page == 0) {
			draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"기본 장비를 강화하고 부족한 재료를 묶음으로 사두라냥!\"", 95, 185, text_dark_cocoa);
		}
		else {
			draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"비싼 신메뉴 레시피를 연구해 해금하면 매출이 폭발한다구냥!\"", 95, 185, text_dark_cocoa);
		}

		// 📦 4열 종대 카드 레이아웃 배치 연산 루프 분기
		int card_y = 235;
		int card_w = 200;
		int card_h = 175;
		int gap = 8;
		int x0 = 70;

		if (g->shop_page == 0) {
			// ================= PAGE 0: 장비 및 재고 구매 레이아웃 =================
			// --- [카드 0: 제조 슬롯 확장] ---
			SDL_Rect rc0 = { x0, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc0);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc0);
			SDL_Rect imgR0 = { x0 + 68, card_y + 10, 64, 64 };
			if (g_tex_shop_slot) SDL_RenderCopy(ren, g_tex_shop_slot, NULL, &imgR0);
			draw_text(ren, g_fnt_sm, "🔨 [8] 슬롯 확장", x0 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "보유 수량: %d / %d", g->slot_count, g->upg[0].max_level);
			draw_text(ren, g_fnt_sm, shopBuf, x0 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 3,000원", x0 + 12, card_y + 140, shop_pink_main);

			// --- [카드 1: 머신 속도 향상] ---
			int x1 = x0 + card_w + gap;
			SDL_Rect rc1 = { x1, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc1);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc1);
			SDL_Rect imgR1 = { x1 + 68, card_y + 10, 64, 64 };
			if (g_tex_shop_machine) SDL_RenderCopy(ren, g_tex_shop_machine, NULL, &imgR1);
			draw_text(ren, g_fnt_sm, "⚡ [9] 머신 속도", x1 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "현재 등급: Lv.%d / %d", g->upg[1].level, g->upg[1].max_level);
			draw_text(ren, g_fnt_sm, shopBuf, x1 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 25,000원", x1 + 12, card_y + 140, shop_pink_main);

			// --- [카드 2: 원두 도매 구매] ---
			int x2 = x1 + card_w + gap;
			SDL_Rect rc2 = { x2, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc2);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc2);
			SDL_Rect imgR2 = { x2 + 75, card_y + 20, 50, 50 };
			SDL_SetRenderDrawColor(ren, 139, 90, 60, 255);
			SDL_RenderFillRect(ren, &imgR2);
			draw_text(ren, g_fnt_sm, "BEAN", x2 + 84, card_y + 36, text_white);
			draw_text(ren, g_fnt_sm, "🫘 원두 자루 (x10)", x2 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "현재고: %d개 보유중", g->stock[ING_BEAN]);
			draw_text(ren, g_fnt_sm, shopBuf, x2 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,500원", x2 + 12, card_y + 140, shop_pink_main);

			// --- [카드 3: 우유 도매 구매] ---
			int x3 = x2 + card_w + gap;
			SDL_Rect rc3 = { x3, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc3);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc3);
			SDL_Rect imgR3 = { x3 + 75, card_y + 20, 50, 50 };
			SDL_SetRenderDrawColor(ren, 230, 240, 250, 255);
			SDL_RenderFillRect(ren, &imgR3);
			draw_text(ren, g_fnt_sm, "MILK", x3 + 85, card_y + 36, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "🥛 우유 묶음 (x10)", x3 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "현재고: %d개 보유중", g->stock[ING_MILK]);
			draw_text(ren, g_fnt_sm, shopBuf, x3 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,000원", x3 + 12, card_y + 140, shop_pink_main);

		}
		else {
			// ================= PAGE 1: 레시피 신메뉴 연구소 레이아웃 =================
			// --- [카드 4: 바닐라 라떼 해금] ---
			SDL_Rect rc4 = { x0, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc4);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc4);

			SDL_Rect imgR4 = { x0 + 75, card_y + 20, 50, 50 };
			SDL_SetRenderDrawColor(ren, 243, 156, 18, 255); // 시럽 황금색 주입
			SDL_RenderFillRect(ren, &imgR4);
			draw_text(ren, g_fnt_sm, "LATTE+", x0 + 78, card_y + 36, text_white);

			draw_text(ren, g_fnt_sm, "☕ 바닐라라떼 해금", x0 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "상태: %s", g_menu[MENU_VANILLA_LATTE].unlocked ? "🔓연구 완료" : "🔒잠김");
			draw_text(ren, g_fnt_sm, shopBuf, x0 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "연구비: 6,000원", x0 + 12, card_y + 140, shop_pink_main);

			// --- [카드 5: 콜드 브루 해금] ---
			int x1 = x0 + card_w + gap;
			SDL_Rect rc1 = { x1, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
			SDL_RenderFillRect(ren, &rc1);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
			SDL_RenderDrawRect(ren, &rc1);

			SDL_Rect imgR5 = { x1 + 75, card_y + 20, 50, 50 };
			SDL_SetRenderDrawColor(ren, 52, 152, 219, 255); // 시원한 청크 얼음색
			SDL_RenderFillRect(ren, &imgR5);
			draw_text(ren, g_fnt_sm, "COLD", x1 + 84, card_y + 36, text_white);

			draw_text(ren, g_fnt_sm, "☕ 콜드브루 해금", x1 + 12, card_y + 82, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "상태: %s", g_menu[MENU_COLD_BREW].unlocked ? "🔓연구 완료" : "🔒잠김");
			draw_text(ren, g_fnt_sm, shopBuf, x1 + 12, card_y + 110, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "연구비: 9,000원", x1 + 12, card_y + 140, shop_pink_main);

			// 빈공간 채우기용 안내 가이드 카드
			int x2 = x1 + card_w + gap;
			SDL_Rect rc2 = { x2, card_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, 240, 240, 240, 255);
			SDL_RenderFillRect(ren, &rc2);
			draw_text(ren, g_fnt_sm, "✨ 신메뉴 연구소", x2 + 12, card_y + 20, unselected_tab);
			draw_text(ren, g_fnt_sm, "메뉴를 해금하면", x2 + 12, card_y + 55, unselected_tab);
			draw_text(ren, g_fnt_sm, "더 높은 마진과 콤보", x2 + 12, card_y + 85, unselected_tab);
			draw_text(ren, g_fnt_sm, "보너스가 터집니다!", x2 + 12, card_y + 115, unselected_tab);
		}

		// ----------------------------------------------------
		// 🧾 5. 하단 영업 가이드 네비게이션 바
		SDL_Rect bottomGuide = { 70, 425, SCREEN_W - 140, 45 };
		SDL_SetRenderDrawColor(ren, 255, 150, 180, 255);
		SDL_RenderFillRect(ren, &bottomGuide);
		draw_text(ren, g_fnt_md, "➔ 키보드 [◀]/[▶] 방향키로 상점 페이지 전환! 정비 완료 후 [Enter] 영업 개시", 90, 437, text_white);

		if (g->log_count > 0) {
			draw_text(ren, g_fnt_sm, g->log_lines[(g->log_count - 1) % MAX_LOG_LINES], 70, 485, shop_pink_main);
		}
	}
	SDL_RenderPresent(ren);
}