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
	g_fnt_sm = TTF_OpenFont("font.ttf", 12);
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

	SDL_Color text_gold = { 253, 203, 110, 255 };
	SDL_Color text_white = { 255, 255, 255, 255 };
	SDL_Color text_coffee_brown = { 115, 80, 60, 255 };

	/* ================= STATE_MAIN: 타이틀 메인 ================= */
	if (g->state == STATE_MAIN) {
		draw_game_background(ren);

		SDL_Rect titleShadow = { SCREEN_W / 2 - 240, 160, 480, 60 };
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(ren, 26, 18, 14, 200);
		SDL_RenderFillRect(ren, &titleShadow);
		draw_text(ren, g_fnt_lg, "☕ 컴파일 커피 (Compile Coffee) ☕", SCREEN_W / 2 - 215, 175, text_gold);

		SDL_Rect btnStart = { 380, 340, 200, 55 };
		SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 255);
		SDL_RenderFillRect(ren, &btnStart);
		SDL_SetRenderDrawColor(ren, 253, 203, 110, 180);
		SDL_RenderDrawRect(ren, &btnStart);
		draw_text(ren, g_fnt_md, "▶ 게임 시작", 432, 356, text_white);

		SDL_Rect btnTutorial = { 380, 420, 200, 55 };
		SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 255);
		SDL_RenderFillRect(ren, &btnTutorial);
		SDL_SetRenderDrawColor(ren, 253, 203, 110, 180);
		SDL_RenderDrawRect(ren, &btnTutorial);
		draw_text(ren, g_fnt_md, "📝 게임 설명", 432, 436, text_white);
	}
	/* ================= STATE_TUTORIAL: 게임 설명 (15일로 스케일 변경) ================= */
	else if (g->state == STATE_TUTORIAL) {
		draw_game_background(ren);

		SDL_Rect tutorialPanel = { 80, 70, SCREEN_W - 160, SCREEN_H - 140 };
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(ren, 33, 24, 18, 245);
		SDL_RenderFillRect(ren, &tutorialPanel);
		SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 255);
		SDL_RenderDrawRect(ren, &tutorialPanel);

		draw_text(ren, g_fnt_lg, "☕ 컴파일 커피 - 완벽 경영 도움말 ☕", 120, 100, text_gold);

		SDL_Color txtGray = { 220, 221, 230, 255 };
		draw_text(ren, g_fnt_md, "1. 바리스타 조작 가이드", 120, 160, text_gold);
		draw_text(ren, g_fnt_sm, "- 마우스로 [에스프레소 머신 슬롯]을 선택 후, 메뉴 아이콘이나 단축키로 음료를 제조합니다.", 140, 190, txtGray);
		draw_text(ren, g_fnt_sm, "- 단축키 연동: Q(아메리카노), W(라떼), E(바닐라라떼), R(콜드브루), T(카라멜), Y(에스프레소)", 140, 215, txtGray);
		draw_text(ren, g_fnt_sm, "- 음료 제조가 [★완료★]되면, 해당 슬롯 지정 후 손님을 마우스로 클릭해 최종 서빙합니다.", 140, 240, txtGray);

		draw_text(ren, g_fnt_md, "2. 손님 맞춤형 성향 공략 및 피크타임", 120, 280, text_gold);
		draw_text(ren, g_fnt_sm, "- [직장인]: 성격이 급해 대기 게이지가 2배 광속 차감됩니다! 최우선 순위로 대접하세요.", 140, 310, txtGray);
		draw_text(ren, g_fnt_sm, "- [미식가]: 까다롭지만 서빙 성공 시 기본 가격의 1.5배 보너스 특급 팁을 투척합니다.", 140, 335, txtGray);
		draw_text(ren, g_fnt_sm, "- [돌발 러시]: 매일 아침 일정 확률로 점심 직장인 대규모 피크타임(스폰 3.2초 단축)이 발동합니다.", 140, 360, txtGray);

		draw_text(ren, g_fnt_md, "3. 최종 승리 마일스톤 조건 (15일 단축 패치)", 120, 400, text_gold);
		draw_text(ren, g_fnt_sm, "- 하루 영업 시간은 단 60초! 총 15일 동안 매장을 알차게 경영해야 마무리가 진행됩니다.", 140, 430, txtGray);
		draw_text(ren, g_fnt_sm, "- 재고 부족 시 긴급 충전 단가 페널티가 발생하니, 상점에서 재료들을 미리 도매 구매하세요.", 140, 455, txtGray);
		draw_text(ren, g_fnt_sm, "★ 최종 목표: 제한 일수 15일 내에 매장 순이익 🪙 1,000,000원 이상을 달성하면 대승리!", 140, 485, text_gold);

		SDL_Color returnTxtColor = { 180, 180, 180, 255 };
		draw_text(ren, g_fnt_md, "[ 화면 아무 곳이나 마우스로 클릭하면 메인 화면으로 돌아갑니다. ]", SCREEN_W / 2 - 240, 525, returnTxtColor);
	}
	/* ================= STATE_PLAYING / STATE_CLOSING: 인게임 영업 ================= */
	else if (g->state == STATE_PLAYING || g->state == STATE_CLOSING) {
		draw_game_background(ren);
		SDL_Rect headerBg = { 10, 10, SCREEN_W - 20, 50 };
		SDL_SetRenderDrawColor(ren, 26, 18, 14, 220);
		SDL_RenderFillRect(ren, &headerBg);
		SDL_SetRenderDrawColor(ren, 90, 70, 58, 255);
		SDL_RenderDrawRect(ren, &headerBg);

		char uiBuf[256];
		sprintf_s(uiBuf, sizeof(uiBuf), "🪙 %d원  [평판: %d/100]", g->balance, g->reputation);
		draw_text(ren, g_fnt_md, uiBuf, 25, 24, text_gold);

		sprintf_s(uiBuf, sizeof(uiBuf), "DAY %02d", g->day);
		draw_text(ren, g_fnt_md, uiBuf, 350, 24, text_white);

		int bar_x = 420; int bar_y = 30; int bar_max_w = 310; int bar_h = 12;
		SDL_Rect timeBarBg = { bar_x, bar_y, bar_max_w, bar_h };
		SDL_SetRenderDrawColor(ren, 64, 48, 38, 255);
		SDL_RenderFillRect(ren, &timeBarBg);

		float time_ratio = (float)g->day_ms / 60000.0f;
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

		SDL_Rect rPauseBtn = { 835, 20, 52, 26 };
		SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 255);
		SDL_RenderFillRect(ren, &rPauseBtn);
		SDL_SetRenderDrawColor(ren, text_gold.r, text_gold.g, text_gold.b, 150);
		SDL_RenderDrawRect(ren, &rPauseBtn);
		draw_text(ren, g_fnt_sm, g->is_paused ? "재개" : "정지", 846, 24, text_white);

		SDL_Rect rHomeBtn = { 892, 20, 32, 26 };
		SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 255);
		SDL_RenderFillRect(ren, &rHomeBtn);
		SDL_SetRenderDrawColor(ren, text_gold.r, text_gold.g, text_gold.b, 150);
		SDL_RenderDrawRect(ren, &rHomeBtn);
		draw_text(ren, g_fnt_sm, "홈", 900, 24, text_white);

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

		if (g->is_paused) {
			SDL_Rect pauseMask = { 0, 0, SCREEN_W, SCREEN_H };
			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(ren, 15, 10, 8, 210);
			SDL_RenderFillRect(ren, &pauseMask);

			SDL_Rect pauseCenterBox = { SCREEN_W / 2 - 250, SCREEN_H / 2 - 45, 500, 90 };
			SDL_SetRenderDrawColor(ren, text_coffee_brown.r, text_coffee_brown.g, text_coffee_brown.b, 240);
			SDL_RenderFillRect(ren, &pauseCenterBox);
			SDL_SetRenderDrawColor(ren, text_gold.r, text_gold.g, text_gold.b, 255);
			SDL_RenderDrawRect(ren, &pauseCenterBox);

			draw_text(ren, g_fnt_md, "⏸️ 매장 운영이 일시 정지되었습니다.", SCREEN_W / 2 - 150, SCREEN_H / 2 - 28, text_gold);
			draw_text(ren, g_fnt_sm, "[우측 상단의 ▶ 재개 버튼이나 단축키 'P'를 누르면 다시 흐릅니다]", SCREEN_W / 2 - 205, SCREEN_H / 2 + 10, text_white);
		}
	}
	/* ================= 💎 STATE_UPGRADE: 상점 대개편 완료 ================= */
	else if (g->state == STATE_UPGRADE) {
		SDL_Color shop_pink_main = { 255, 121, 198, 255 };
		SDL_Color text_dark_cocoa = { 85, 55, 65, 255 };
		SDL_Color light_pink_card = { 255, 242, 245, 255 };
		SDL_Color pink_border = { 255, 182, 193, 255 };
		char shopBuf[256];

		SDL_Rect shopBg = { 40, 40, SCREEN_W - 80, SCREEN_H - 120 };
		SDL_SetRenderDrawColor(ren, 255, 218, 224, 255);
		SDL_RenderFillRect(ren, &shopBg);
		SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		SDL_RenderDrawRect(ren, &shopBg);

		sprintf_s(shopBuf, sizeof(shopBuf), "💖 컴파일 커피 정비소 - DAY %d 💖", g->day);
		draw_text(ren, g_fnt_lg, shopBuf, 70, 55, shop_pink_main);

		sprintf_s(shopBuf, sizeof(shopBuf), "보유 자산: 🪙 %d원  |  🫘:%d  🥛:%d  🍯:%d  🥛+:%d  🧊:%d",
			g->balance, g->stock[ING_BEAN], g->stock[ING_MILK], g->stock[ING_SYRUP], g->stock[ING_CREAM], g->stock[ING_ICE]);
		draw_text(ren, g_fnt_md, shopBuf, 70, 95, shop_pink_main);

		SDL_Rect tab1 = { 70, 130, 180, 30 };
		if (g->shop_page == 0) SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		else SDL_SetRenderDrawColor(ren, 200, 170, 180, 255);
		SDL_RenderFillRect(ren, &tab1);
		draw_text(ren, g_fnt_sm, "[◀] 1. 장비 & 재고 구매", 85, 137, text_white);

		SDL_Rect tab2 = { 260, 130, 180, 30 };
		if (g->shop_page == 1) SDL_SetRenderDrawColor(ren, shop_pink_main.r, shop_pink_main.g, shop_pink_main.b, 255);
		else SDL_SetRenderDrawColor(ren, 200, 170, 180, 255);
		SDL_RenderFillRect(ren, &tab2);
		draw_text(ren, g_fnt_sm, "[▶] 2. 메뉴 레시피 해금", 275, 137, text_white);

		SDL_Rect NPCBubble = { 70, 170, SCREEN_W - 140, 50 };
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderFillRect(ren, &NPCBubble);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &NPCBubble);

		if (g->shop_page == 0) {
			draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"재고 품목이 늘어났다냥! 시럽, 크림, 얼음도 든든하게 비축하라냥!\"", 95, 185, text_dark_cocoa);
		}
		else {
			draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"최종 사니처 메뉴 '카라멜마끼아또' 연구 슬롯이 열렸다냥! 연구하라냥!\"", 95, 185, text_dark_cocoa);
		}

		int gap = 8; int card_w = 175; int card_h = 160; int x0 = 65;

		if (g->shop_page == 0) {
			SDL_Rect rc0 = { x0, 230, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc0);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc0);
			draw_text(ren, g_fnt_sm, "🔨 슬롯 확장", x0 + 10, 240, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "보유: %d/%d", g->slot_count, g->upg[0].max_level); draw_text(ren, g_fnt_sm, shopBuf, x0 + 10, 270, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 3,000원", x0 + 10, 330, shop_pink_main);

			int x1 = x0 + card_w + gap; SDL_Rect rc1 = { x1, 230, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc1);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc1);
			draw_text(ren, g_fnt_sm, "⚡ 머신 속도", x1 + 10, 240, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "등급: Lv.%d/%d", g->upg[1].level, g->upg[1].max_level); draw_text(ren, g_fnt_sm, shopBuf, x1 + 10, 270, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 25,000원", x1 + 10, 330, shop_pink_main);

			// 🎯 [재고 가시성 완치]: 어색한 "고:" 머리띠를 다 밀어버리고 정식 "재고: x개" 레이아웃 이식
			int x2 = x1 + card_w + gap; SDL_Rect rc2 = { x2, 230, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc2);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc2);
			draw_text(ren, g_fnt_sm, "🫘 원두 자루(x10)", x2 + 10, 240, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "재고: %d개 보유중", g->stock[ING_BEAN]); draw_text(ren, g_fnt_sm, shopBuf, x2 + 10, 270, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,500원", x2 + 10, 330, shop_pink_main);

			int x3 = x2 + card_w + gap; SDL_Rect rc3 = { x3, 230, card_w, card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc3);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc3);
			draw_text(ren, g_fnt_sm, "🥛 우유 묶음(x10)", x3 + 10, 240, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "재고: %d개 보유중", g->stock[ING_MILK]); draw_text(ren, g_fnt_sm, shopBuf, x3 + 10, 270, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,000원", x3 + 10, 330, shop_pink_main);

			int y_row2 = 400; int card_h2 = 110;

			SDL_Rect rc6 = { x0, y_row2, card_w, card_h2 };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc6);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc6);
			draw_text(ren, g_fnt_sm, "🍯 시럽 보틀(x10)", x0 + 10, y_row2 + 10, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "재고: %d개 보유중", g->stock[ING_SYRUP]); draw_text(ren, g_fnt_sm, shopBuf, x0 + 10, y_row2 + 35, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,200원", x0 + 10, y_row2 + 75, shop_pink_main);

			SDL_Rect rc7 = { x1, y_row2, card_w, card_h2 };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc7);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc7);
			draw_text(ren, g_fnt_sm, "🥛+ 휘핑 크림(x10)", x1 + 10, y_row2 + 10, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "재고: %d개 보유중", g->stock[ING_CREAM]); draw_text(ren, g_fnt_sm, shopBuf, x1 + 10, y_row2 + 35, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 1,400원", x1 + 10, y_row2 + 75, shop_pink_main);

			SDL_Rect rc8 = { x2, y_row2, card_w, card_h2 };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc8);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc8);
			draw_text(ren, g_fnt_sm, "🧊 단단한 얼음(x10)", x2 + 10, y_row2 + 10, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "재고: %d개 보유중", g->stock[ING_ICE]); draw_text(ren, g_fnt_sm, shopBuf, x2 + 10, y_row2 + 35, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "비용: 800원", x2 + 10, y_row2 + 75, shop_pink_main);
		}
		else {
			int card_y = 235; int big_card_w = 230; int big_card_h = 175; int big_gap = 15;

			SDL_Rect rc4 = { x0, card_y, big_card_w, big_card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc4);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc4);
			draw_text(ren, g_fnt_sm, "☕ 바닐라라떼 레시피", x0 + 15, card_y + 15, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "연구: %s", g_menu[MENU_VANILLA_LATTE].unlocked ? "🔓 완료" : "🔒 잠김"); draw_text(ren, g_fnt_sm, shopBuf, x0 + 15, card_y + 55, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "연구비: 6,000원", x0 + 15, card_y + 115, shop_pink_main);

			int x1 = x0 + big_card_w + big_gap; SDL_Rect rc5 = { x1, card_y, big_card_w, big_card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc5);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc5);
			draw_text(ren, g_fnt_sm, "☕ 콜드브루 레시피", x1 + 15, card_y + 15, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "연구: %s", g_menu[MENU_COLD_BREW].unlocked ? "🔓 완료" : "🔒 잠김"); draw_text(ren, g_fnt_sm, shopBuf, x1 + 15, card_y + 55, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "연구비: 9,000원", x1 + 15, card_y + 115, shop_pink_main);

			int x2 = x1 + big_card_w + big_gap; SDL_Rect rc9 = { x2, card_y, big_card_w, big_card_h };
			SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255); SDL_RenderFillRect(ren, &rc9);
			SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255); SDL_RenderDrawRect(ren, &rc9);
			draw_text(ren, g_fnt_sm, "✨ 카라멜마끼아또 해금", x2 + 15, card_y + 15, shop_pink_main);
			sprintf_s(shopBuf, sizeof(shopBuf), "연구: %s", g_menu[MENU_CARAMEL_MAC].unlocked ? "🔓 완료" : "🔒 잠김"); draw_text(ren, g_fnt_sm, shopBuf, x2 + 15, card_y + 55, text_dark_cocoa);
			draw_text(ren, g_fnt_sm, "연구비: 12,000원", x2 + 15, card_y + 115, shop_pink_main);
		}

		SDL_Rect bottomGuide = { 70, 520, SCREEN_W - 140, 35 };
		SDL_SetRenderDrawColor(ren, 255, 150, 180, 255); SDL_RenderFillRect(ren, &bottomGuide);
		draw_text(ren, g_fnt_sm, "➔ 키보드 [◀]/[▶] 방향키로 상점 페이지 전환! 정비 완료 후 [Enter] 영업 개시", 90, 530, text_white);

		// 🎯 [잔상 로그 파괴 완치]: 상점 모듈 내부일 땐 하단에 인게임용 겹침 로그가 절대 뜨지 않도록 완벽 필터 아웃!
	}
	SDL_RenderPresent(ren);
}