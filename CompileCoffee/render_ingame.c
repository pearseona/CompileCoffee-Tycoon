#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../common.h"     
#include "../render.h"
#include "../customer.h"

extern SDL_Texture* g_tex_customers[3];
extern SDL_Texture* g_tex_barista;
extern SDL_Texture* g_tex_station;
extern SDL_Texture* g_tex_background;
extern SDL_Texture* g_tex_menus[6];
extern TTF_Font* g_fnt_sm;
extern TTF_Font* g_fnt_md;

/* 게임 배경 */
void draw_game_background(SDL_Renderer* ren) {
	if (g_tex_background) {
		SDL_Rect bgRect = { 0, 0, SCREEN_W, SCREEN_H };
		SDL_RenderCopy(ren, g_tex_background, NULL, &bgRect);
	}
	else {
		SDL_SetRenderDrawColor(ren, 250, 234, 222, 255);
		SDL_RenderClear(ren);
	}
}

/* 음료 컵 */
void draw_procedural_cup(SDL_Renderer* ren, int x, int y, int w, int h, MenuID menu, bool completed) {

	SDL_Rect body = { x + w / 6, y + h / 3, w * 2 / 3, h * 3 / 5 };

	// 컵 내용물
	if (menu == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 109, 76, 65, 255);
	else if (menu == MENU_LATTE) SDL_SetRenderDrawColor(ren, 222, 184, 135, 255);
	else if (menu == MENU_VANILLA_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
	else if (menu == MENU_COLD_BREW) SDL_SetRenderDrawColor(ren, 93, 64, 55, 255);
	else if (menu == MENU_CARAMEL_MAC) SDL_SetRenderDrawColor(ren, 210, 105, 30, 255);

	else SDL_SetRenderDrawColor(ren, 62, 39, 35, 255);

	SDL_RenderFillRect(ren, &body);
	SDL_SetRenderDrawColor(ren, 60, 45, 35, 255);
	SDL_RenderDrawRect(ren, &body);
	SDL_Rect handle = { x + w * 5 / 6 - 2, y + h / 2 - 2, w / 6, h / 4 };
	SDL_SetRenderDrawColor(ren, 60, 45, 35, 255);
	SDL_RenderDrawRect(ren, &handle);

	if (completed) {
		SDL_Rect foam = { x + w / 6 + 2, y + h / 3 + 2, w * 2 / 3 - 4, 4 };
		SDL_SetRenderDrawColor(ren, 255, 250, 240, 255);
		SDL_RenderFillRect(ren, &foam);
	}
}

/* 대기 중인 손님 3명 화면에 출력 */
void draw_customer_queue(SDL_Renderer* ren, Game* g) {

	int spots_x[3] = { 180, 440, 700 };
	int cy = 165;

	// 빈 대기 슬롯
	for (int i = 0; i < 3; i++) {
		SDL_Rect slotFrame = { spots_x[i], cy, 135, 160 };
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 25);
		SDL_RenderFillRect(ren, &slotFrame);
		SDL_SetRenderDrawColor(ren, 115, 80, 60, 60);
		SDL_RenderDrawRect(ren, &slotFrame);
	}

	// 각 손님 렌더링
	for (int i = 0; i < 3; i++) {
		if (g->queue[i].active) {
			Customer* c = &g->queue[i];

			// 손님 캐릭터
			SDL_Rect dst = { spots_x[i], cy, 135, 160 };
			if (g_tex_customers[c->type]) {
				SDL_RenderCopy(ren, g_tex_customers[c->type], NULL, &dst);
			}

			SDL_Rect textCard = { spots_x[i] + 10, cy + 170, 115, 24 };
			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(ren, 20, 15, 12, 230);
			SDL_RenderFillRect(ren, &textCard);
			SDL_SetRenderDrawColor(ren, 253, 203, 110, 150);
			SDL_RenderDrawRect(ren, &textCard);

			// 손님 명찰
			char specBuf[64];
			SDL_Color cText_color = { 255, 255, 255, 255 };
			if (c->type == CUST_WORKER) {
				sprintf_s(specBuf, sizeof(specBuf), "[🚨급함] 직장인");
				cText_color = (SDL_Color){ 255, 120, 120, 255 };
			}
			else if (c->type == CUST_FOODIE) {
				sprintf_s(specBuf, sizeof(specBuf), "[👑팁!] 미식가");
				cText_color = (SDL_Color){ 253, 203, 110, 255 };
			}
			else {
				sprintf_s(specBuf, sizeof(specBuf), "[🎓할인!] 학 생");
				cText_color = (SDL_Color){ 120, 230, 255, 255 };
			}
			draw_text(ren, g_fnt_sm, specBuf, spots_x[i] + 18, cy + 174, cText_color);

			// 주문 메뉴 말풍선
			int bubble_w = 90; int bubble_h = 70;
			int bx = spots_x[i] + 70; int by = cy - 65;
			SDL_Rect bubbleRect = { bx, by, bubble_w, bubble_h };

			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(ren, 245, 246, 250, 245);
			SDL_RenderFillRect(ren, &bubbleRect);
			SDL_SetRenderDrawColor(ren, 113, 128, 147, 255);
			SDL_RenderDrawRect(ren, &bubbleRect);

			// 인내심 게이지 바 계산
			double patience_ratio = (double)c->patience_ms / c->patience_max;
			if (patience_ratio < 0.0) patience_ratio = 0.0;
			if (patience_ratio > 1.0) patience_ratio = 1.0;

			int gauge_x = bx + 5; int gauge_y = by + 6; int gauge_w = 7; int gauge_max_h = bubble_h - 12;
			int gauge_curr_h = (int)(gauge_max_h * patience_ratio);
			SDL_Rect gaugeBg = { gauge_x, gauge_y, gauge_w, gauge_max_h };
			SDL_SetRenderDrawColor(ren, 87, 101, 116, 255);
			SDL_RenderFillRect(ren, &gaugeBg);

			SDL_Rect gaugeFill = { gauge_x, gauge_y + (gauge_max_h - gauge_curr_h), gauge_w, gauge_curr_h };
			if (patience_ratio > 0.5) SDL_SetRenderDrawColor(ren, 46, 204, 113, 255);
			else if (patience_ratio > 0.25) SDL_SetRenderDrawColor(ren, 241, 196, 15, 255);
			else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
			SDL_RenderFillRect(ren, &gaugeFill);

			// 주문 음료 아이콘 
			SDL_Rect iconRect = { bx + 22, by + 10, 45, 45 };
			MenuID order_menu = c->order;
			if (g_tex_menus[order_menu]) {
				SDL_RenderCopy(ren, g_tex_menus[order_menu], NULL, &iconRect);
			}
			else {
				draw_procedural_cup(ren, bx + 22, by + 10, 45, 45, order_menu, true);
			}

			SDL_Color nameColor = { 47, 53, 66, 255 };
			draw_text(ren, g_fnt_sm, g_menu[order_menu].name, bx + 16, by + bubble_h - 14, nameColor);

			int bar_w = 120; int bar_h = 6;
			int bx_bar = spots_x[i] + 7; int by_bar = cy + 202;
			SDL_Rect barBg = { bx_bar, by_bar, bar_w, bar_h };
			SDL_SetRenderDrawColor(ren, 50, 40, 35, 255);
			SDL_RenderFillRect(ren, &barBg);

			float ratio = (float)c->patience_ms / (float)c->patience_max;
			if (ratio < 0) ratio = 0;
			int curr_w = (int)(bar_w * ratio);
			SDL_Rect barFill = { bx_bar, by_bar, curr_w, bar_h };

			if (ratio > 0.5f) SDL_SetRenderDrawColor(ren, 46, 204, 113, 255);
			else if (ratio > 0.2f) SDL_SetRenderDrawColor(ren, 241, 196, 15, 255);
			else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
			SDL_RenderFillRect(ren, &barFill);

			// 선택 시 두꺼운 테두리 
			if (g->sel_cust == i) {
				SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
				for (int t = 0; t < 3; t++) {
					SDL_Rect thickOutline = { dst.x - t, dst.y - t, dst.w + (t * 2), dst.h + (t * 2) };
					SDL_RenderDrawRect(ren, &thickOutline);
				}
			}
		}
	}
}

/* 커피 머신 슬롯 및 제조 */
void draw_barista_slots(SDL_Renderer* ren, Game* g) {

	SDL_Rect machineBg = { 50, 360, 250, 160 };
	SDL_SetRenderDrawColor(ren, 127, 140, 141, 255);
	SDL_RenderFillRect(ren, &machineBg);
	SDL_SetRenderDrawColor(ren, 189, 195, 199, 255);
	SDL_RenderDrawRect(ren, &machineBg);
	SDL_Rect machineHeader = { 50, 360, 250, 25 };
	SDL_SetRenderDrawColor(ren, 44, 62, 80, 255);
	SDL_RenderFillRect(ren, &machineHeader);
	SDL_Color white = { 255, 255, 255, 255 };
	draw_text(ren, g_fnt_sm, "☕ COFFEE MACHINE", 65, 365, white);

	// 슬롯 개수만큼 반복
	for (int i = 0; i < g->slot_count; i++) {
		int sx = 70 + i * 80; int sy = 400; SDL_Rect cupSpot = { sx, sy, 60, 80 };
		if (g->sel_slot == i) {
			SDL_Rect selSlotOutline = { sx - 4, sy - 4, 68, 88 };
			SDL_SetRenderDrawColor(ren, 254, 202, 87, 255);
			SDL_RenderDrawRect(ren, &selSlotOutline);
		}
		SDL_Rect nozzle = { sx + 22, sy - 12, 16, 12 };
		SDL_SetRenderDrawColor(ren, 52, 73, 94, 255);
		SDL_RenderFillRect(ren, &cupSpot);

		SlotState state = g->slots[i].state;

		// 빈 슬롯 상태
		if (state == SLOT_EMPTY) {
			SDL_SetRenderDrawColor(ren, 90, 105, 120, 180);
			SDL_RenderFillRect(ren, &cupSpot);
			SDL_Color textGray = { 200, 214, 229, 255 };
			char numStr[128];
			sprintf_s(numStr, sizeof(numStr), "#%d 빈슬롯", i + 1);
			draw_text(ren, g_fnt_sm, numStr, sx + 5, sy + 32, textGray);
		}

		// 제조 중 상태
		else if (state == SLOT_BREWING) {
			SDL_SetRenderDrawColor(ren, 149, 175, 192, 255);
			SDL_RenderFillRect(ren, &cupSpot);
			draw_procedural_cup(ren, sx, sy + 20, 60, 60, g->slots[i].menu, false);
			Uint32 tick = SDL_GetTicks();
			if ((tick / 150) % 2 == 0) {
				SDL_Rect drip = { sx + 28, sy, 4, 20 };
				SDL_SetRenderDrawColor(ren, 139, 69, 19, 255);
				SDL_RenderFillRect(ren, &drip);
			}

			// 제조 진행률 계산
			int bar_max_w = 50; 
			float brew_ratio = (float)g->slots[i].elapsed_ms / g->slots[i].required_ms;
			if (brew_ratio > 1.0f) brew_ratio = 1.0f;
			int bar_curr_w = (int)(bar_max_w * brew_ratio);

			SDL_Rect progressBg = { sx + 5, sy + 5, bar_max_w, 6 };
			SDL_SetRenderDrawColor(ren, 87, 101, 116, 255);
			SDL_RenderFillRect(ren, &progressBg);
			SDL_Rect progressFill = { sx + 5, sy + 5, bar_curr_w, 6 };
			SDL_SetRenderDrawColor(ren, 9, 132, 227, 255);
			SDL_RenderFillRect(ren, &progressFill);
		}

		// 제조 완료
		else if (state == SLOT_DONE) {
			SDL_SetRenderDrawColor(ren, 108, 92, 231, 100);
			SDL_RenderFillRect(ren, &cupSpot);
			draw_procedural_cup(ren, sx, sy + 20, 60, 60, g->slots[i].menu, true);

			// 김 나는 효과(Steam)
			Uint32 tick = SDL_GetTicks(); int steam_offset = (tick / 200) % 3;
			SDL_SetRenderDrawColor(ren, 255, 255, 255, 180);
			SDL_RenderDrawLine(ren, sx + 20 + steam_offset, sy + 15, sx + 20 + steam_offset, sy + 5);
			SDL_Color textGreen = { 76, 209, 55, 255 };
			draw_text(ren, g_fnt_sm, "★완료★", sx + 8, sy + 2, textGreen);
		}
	}

	// 하단 메뉴 선택
	if (g->sel_slot >= 0 && g->sel_slot < g->slot_count && g->slots[g->sel_slot].state == SLOT_EMPTY) {
		int mx_start = 320; int my_start = 410;
		SDL_Rect menuPanel = { mx_start, my_start, 355, 85 };
		SDL_SetRenderDrawColor(ren, 75, 58, 46, 245);
		SDL_RenderFillRect(ren, &menuPanel);
		SDL_SetRenderDrawColor(ren, 253, 203, 110, 255);
		SDL_RenderDrawRect(ren, &menuPanel);

		SDL_Color goldText = { 253, 203, 110, 255 };
		draw_text(ren, g_fnt_sm, "제조할 음료를 선택하세요 (키보드 Q,W,E,R,T,Y)", mx_start + 15, my_start + 5, goldText);

		char* hotkeys[6] = { "Q", "W", "E", "R", "T", "Y" };
		const char* full_menu_names[6] = { "아메리카노", "카페라떼", "바닐라", "콜드브루", "카라멜마끼아또", "에스프레소" };

		// 메뉴 해금 상태에 따른 아이콘
		for (int i = 0; i < 6; i++) {
			int bx = mx_start + 10 + i * 56; int by = my_start + 26; SDL_Rect btnRect = { bx, by, 45, 45 };
		
			if (g_menu[i].unlocked) {
				SDL_SetRenderDrawColor(ren, 115, 96, 83, 255);
				SDL_RenderFillRect(ren, &btnRect);
				if (g_tex_menus[i]) SDL_RenderCopy(ren, g_tex_menus[i], NULL, &btnRect);
				else draw_procedural_cup(ren, bx, by, 45, 45, (MenuID)i, true);

				SDL_Color wt = { 255, 255, 255, 140 };
				draw_text(ren, g_fnt_sm, hotkeys[i], bx + 4, by + 2, wt);

				SDL_Color txtGuideColor = { 245, 246, 250, 255 };

				int text_x_pos = bx + 2;
				if (i == 0) text_x_pos = bx - 10;
				else if (i == 1) text_x_pos = bx - 5;
				else if (i == 2) text_x_pos = bx - 10;
				else if (i == 3) text_x_pos = bx - 5;
				else if (i == 4) text_x_pos = bx - 18;
				else if (i == 5) text_x_pos = bx - 5;

				draw_text(ren, g_fnt_sm, full_menu_names[i], text_x_pos, by + 49, txtGuideColor);
			}
			else {
				// 미해금 메뉴
				SDL_SetRenderDrawColor(ren, 50, 40, 35, 255);
				SDL_RenderFillRect(ren, &btnRect);
				SDL_Color lockGray = { 150, 140, 135, 255 };
				draw_text(ren, g_fnt_sm, "🔒", bx + 14, by + 12, lockGray);
			}
		}
	}

	// 슬롯 상세 정보
	else {
		int mx_start = 320; int my_start = 410;
		SDL_Rect infoPanel = { mx_start, my_start, 355, 85 };
		SDL_SetRenderDrawColor(ren, 60, 47, 38, 200);
		SDL_RenderFillRect(ren, &infoPanel);
		SDL_Color tipColor = { 245, 246, 250, 255 };
		char tipBuf[128];
		if (g->sel_slot >= 0 && g->slots[g->sel_slot].state == SLOT_BREWING) {
			sprintf_s(tipBuf, sizeof(tipBuf), "[슬롯 %d] %s 제조 중...", g->sel_slot + 1, g_menu[g->slots[g->sel_slot].menu].name);
			draw_text(ren, g_fnt_md, tipBuf, mx_start + 20, my_start + 18, tipColor);
		}
		else if (g->sel_slot >= 0 && g->slots[g->sel_slot].state == SLOT_DONE) {
			sprintf_s(tipBuf, sizeof(tipBuf), "[슬롯 %d] %s 완성 완료!", g->sel_slot + 1, g_menu[g->slots[g->sel_slot].menu].name);
			draw_text(ren, g_fnt_md, tipBuf, mx_start + 20, my_start + 18, tipColor);
		}
		else {
			draw_text(ren, g_fnt_md, "제조할 머신의 슬롯을 마우스로 선택하세요.", mx_start + 20, my_start + 30, tipColor);
		}
	}

	// 우측 하단 재고 상황
	int stock_x = 690; int stock_y = 410; SDL_Rect stockPanel = { stock_x, stock_y, 250, 85 };
	SDL_SetRenderDrawColor(ren, 48, 57, 82, 220);
	SDL_RenderFillRect(ren, &stockPanel);
	SDL_Color wt = { 255, 255, 255, 255 };
	draw_text(ren, g_fnt_sm, "재고 상황 (부족 시 제조 불가)", stock_x + 10, stock_y + 5, wt);

	SDL_Color stockColors[5] = { {110, 76, 65, 255}, {255, 255, 255, 255}, {254, 202, 87, 255}, {255, 223, 230, 255}, {116, 185, 255, 255} };
	const char* stockShortNames[5] = { "원두", "우유", "시럽", "크림", "얼음" };

	for (int i = 0; i < 5; i++) {
		int ix = stock_x + 8 + i * 48;
		int iy = stock_y + 28;
		SDL_Rect iconB = { ix, iy, 38, 22 };

		SDL_SetRenderDrawColor(ren, stockColors[i].r, stockColors[i].g, stockColors[i].b, 255);
		SDL_RenderFillRect(ren, &iconB);

		SDL_Color txtW = { 255, 255, 255, 255 };
		draw_text(ren, g_fnt_sm, stockShortNames[i], ix + 4, iy + 2, (i == 1) ? (SDL_Color) { 0, 0, 0, 255 } : txtW);

		char countStr[16];
		sprintf_s(countStr, sizeof(countStr), "x%d", g->stock[i]);
		SDL_Color countColor = (g->stock[i] <= 3) ? (SDL_Color) { 231, 76, 60, 255 } : (SDL_Color) { 220, 221, 230, 255 };
		draw_text(ren, g_fnt_sm, countStr, ix + 6, iy + 25, countColor);
	}
}