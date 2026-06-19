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

		sprintf_s(shopBuf, sizeof(shopBuf), "보유 자산: 🪙 %d원  |  제조 슬롯: %d개  |  머신 속도 속도: Lv.%d", g->balance, g->slot_count, g->upg[1].level);
		draw_text(ren, g_fnt_md, shopBuf, 70, 115, shop_pink_main);

		// ----------------------------------------------------
		// 💬 2. 와이어프레임 반영: 점장 고양이의 말풍선 대사창 (상단 가로 고정)
		SDL_Rect NPCBubble = { 70, 160, SCREEN_W - 140, 65 };
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); // 흰색 말풍선 본체
		SDL_RenderFillRect(ren, &NPCBubble);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &NPCBubble);

		draw_text(ren, g_fnt_md, "👩 [점장 고양이]: \"오늘 하루도 수고했어! 모은 돈으로 매장을 더 이쁘게 키워보자구냥!\"", 95, 181, text_dark_cocoa);

		// ----------------------------------------------------
		// 📦 3. 와이어프레임 반영: 2열 가로 분할 아이템 레이아웃 (좌 / 우 정밀 정렬)
		int card_y = 250;
		int card_w = (SCREEN_W - 170) / 2; // 중앙 공백 30px 제외 2열 가로분할 수식 계산
		int card_h = 145;

		// 🔨 [좌측 카드: 제조 슬롯 확장 플레이트]
		SDL_Rect leftCard = { 70, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &leftCard);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &leftCard);

		// 좌측 카드 내부 픽셀 망치 이미지 바인딩 (64x64 크기 배치)
		SDL_Rect imgRect1 = { 85, card_y + 15, 64, 64 };
		if (g_tex_shop_slot) {
			SDL_RenderCopy(ren, g_tex_shop_slot, NULL, &imgRect1);
		}
		// 우측으로 밀어서 겹침 방지 (X: 165)
		draw_text(ren, g_fnt_md, "[8] 제조 슬롯 확장", 165, card_y + 15, shop_pink_main);
		draw_text(ren, g_fnt_sm, "바리스타 고용 (동시 제조 가능)", 165, card_y + 45, text_dark_cocoa);
		sprintf_s(shopBuf, sizeof(shopBuf), "현재 슬롯 수: %d / %d", g->slot_count, g->upg[0].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, 165, card_y + 70, text_dark_cocoa);
		draw_text(ren, g_fnt_md, "비용: 3,000원", 165, card_y + 105, shop_pink_main);


		// ⚡ [우측 카드: 고속 커피 머신 도입 플레이트]
		int right_x = 70 + card_w + 30; // 좌측 카드 끝 + 마진 30px
		SDL_Rect rightCard = { right_x, card_y, card_w, card_h };
		SDL_SetRenderDrawColor(ren, light_pink_card.r, light_pink_card.g, light_pink_card.b, 255);
		SDL_RenderFillRect(ren, &rightCard);
		SDL_SetRenderDrawColor(ren, pink_border.r, pink_border.g, pink_border.b, 255);
		SDL_RenderDrawRect(ren, &rightCard);

		// 우측 카드 내부 픽셀 번개 모카포트 이미지 바인딩 (64x64 크기)
		SDL_Rect imgRect2 = { right_x + 15, card_y + 15, 64, 64 };
		if (g_tex_shop_machine) {
			SDL_RenderCopy(ren, g_tex_shop_machine, NULL, &imgRect2);
		}
		// 우측으로 밀어서 정렬 (X: right_x + 95)
		draw_text(ren, g_fnt_md, "[9] 고속 머신 도입", right_x + 95, card_y + 15, shop_pink_main);
		draw_text(ren, g_fnt_sm, "음료 제조 속도 15% 가속화", right_x + 95, card_y + 45, text_dark_cocoa);
		sprintf_s(shopBuf, sizeof(shopBuf), "현재 등급: Lv.%d / %d", g->upg[1].level, g->upg[1].max_level);
		draw_text(ren, g_fnt_sm, shopBuf, right_x + 95, card_y + 70, text_dark_cocoa);
		draw_text(ren, g_fnt_md, "비용: 25,000원", right_x + 95, card_y + 105, shop_pink_main);

		// ----------------------------------------------------
		// 🧾 4. 하단 영업 가이드 네비게이션 바 (딸기 초코 테마 바)
		SDL_Rect bottomGuide = { 70, 415, SCREEN_W - 140, 45 };
		SDL_SetRenderDrawColor(ren, 255, 150, 180, 255);
		SDL_RenderFillRect(ren, &bottomGuide);
		draw_text(ren, g_fnt_md, "➔ 정비를 마쳤다면 [Enter] 키를 눌러 다음 날 영업을 시작합니다!", 95, 427, text_white);

		// 실시간 피드백 로그 정렬 알림창 연동
		if (g->log_count > 0) {
			draw_text(ren, g_fnt_sm, g->log_lines[(g->log_count - 1) % MAX_LOG_LINES], 70, 485, shop_pink_main);
		}
	}
	SDL_RenderPresent(ren);
}