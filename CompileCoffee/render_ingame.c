#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../common.h"     // 🎯 파일 꼬임 방지: 결합 주소 상대 기호 제거 연동 마스터링
#include "../render.h"
#include "../customer.h"

extern SDL_Texture* g_tex_customers[3];
extern SDL_Texture* g_tex_barista;
extern SDL_Texture* g_tex_station;
extern SDL_Texture* g_tex_background;
extern SDL_Texture* g_tex_menus[6];
extern TTF_Font* g_fnt_sm;
extern TTF_Font* g_fnt_md;

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

void draw_procedural_cup(SDL_Renderer* ren, int x, int y, int w, int h, MenuID menu, bool completed) {
	SDL_Rect body = { x + w / 6, y + h / 3, w * 2 / 3, h * 3 / 5 };
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

void draw_customer_queue(SDL_Renderer* ren, Game* g) {
	int spots_x[3] = { 180, 440, 700 };
	int spot_y = 210; int card_w = 135; int card_h = 160;

	for (int i = 0; i < 3; i++) {
		int cx = spots_x[i];
		SDL_Rect frameRect = { cx, spot_y, card_w, card_h };
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 45);
		SDL_RenderFillRect(ren, &frameRect);
		SDL_SetRenderDrawColor(ren, 139, 106, 80, 120);
		SDL_RenderDrawRect(ren, &frameRect);
	}

	for (int i = 0; i < 3; i++) {
		int cx = spots_x[i];
		if (g->queue[i].active == 1) {
			double patience_ratio = (double)g->queue[i].patience_ms / g->queue[i].patience_max;
			if (patience_ratio < 0.0) patience_ratio = 0.0;
			if (patience_ratio > 1.0) patience_ratio = 1.0;

			if (g->sel_cust == i) {
				SDL_Rect selOutline = { cx - 6, spot_y - 6, card_w + 12, card_h + 12 };
				Uint32 tick = SDL_GetTicks();
				if ((tick / 250) % 2 == 0) {
					SDL_SetRenderDrawColor(ren, 253, 203, 110, 255);
					SDL_RenderDrawRect(ren, &selOutline);
				}
			}
			SDL_Rect customerCard = { cx, spot_y, card_w, card_h };
			int c_type = (int)g->queue[i].type;
			if (c_type >= 0 && c_type < 3 && g_tex_customers[c_type]) {
				SDL_RenderCopy(ren, g_tex_customers[c_type], NULL, &customerCard);
			}
			int bubble_w = 90; int bubble_h = 70; int bx = cx + 70; int by = spot_y - 80;
			SDL_Rect bubbleRect = { bx, by, bubble_w, bubble_h };
			SDL_SetRenderDrawColor(ren, 245, 246, 250, 255);
			SDL_RenderFillRect(ren, &bubbleRect);
			SDL_SetRenderDrawColor(ren, 113, 128, 147, 255);
			SDL_RenderDrawRect(ren, &bubbleRect);

			// 🎯 [시각 고도화]: 머리 위의 탭 인터랙션에 성향 타이틀 표기
			char tagStr[32] = "";
			SDL_Color tagColor = { 47, 53, 66, 255 };

			if (c_type == CUST_WORKER) {
				sprintf_s(tagStr, sizeof(tagStr), "[급함] 직장인");
				tagColor = (SDL_Color){ 231, 76, 60, 255 }; // 직장인은 긴장감 도는 레드
			}
			else if (c_type == CUST_FOODIE) {
				sprintf_s(tagStr, sizeof(tagStr), "★VIP★ 미식가");
				tagColor = (SDL_Color){ 243, 156, 18, 255 }; // 미식가는 프리미엄 골드
			}
			else if (c_type == CUST_STUDENT) {
				sprintf_s(tagStr, sizeof(tagStr), "[할인] 학생");
				tagColor = (SDL_Color){ 46, 204, 113, 255 }; // 학생은 청량한 그린
			}

			// 태그를 말풍선 위 좌표에 드로우
			draw_text(ren, g_fnt_sm, tagStr, cx + 5, spot_y - 97, tagColor);

			int gauge_x = bx + 4; int gauge_y = by + 5; int gauge_w = 7; int gauge_max_h = bubble_h - 10;
			int gauge_curr_h = (int)(gauge_max_h * patience_ratio);
			SDL_Rect gaugeBg = { gauge_x, gauge_y, gauge_w, gauge_max_h };
			SDL_SetRenderDrawColor(ren, 87, 101, 116, 255);
			SDL_RenderFillRect(ren, &gaugeBg);
			SDL_Rect gaugeFill = { gauge_x, gauge_y + (gauge_max_h - gauge_curr_h), gauge_w, gauge_curr_h };
			if (patience_ratio > 0.5) SDL_SetRenderDrawColor(ren, 46, 204, 113, 255);
			else if (patience_ratio > 0.25) SDL_SetRenderDrawColor(ren, 241, 196, 15, 255);
			else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
			SDL_RenderFillRect(ren, &gaugeFill);

			SDL_Rect iconRect = { bx + 22, by + 12, 45, 45 };
			MenuID order_menu = g->queue[i].order;
			if (g_tex_menus[order_menu]) SDL_RenderCopy(ren, g_tex_menus[order_menu], NULL, &iconRect);
			else draw_procedural_cup(ren, bx + 22, by + 12, 45, 45, order_menu, true);

			SDL_Color nameColor = { 47, 53, 66, 255 };
			draw_text(ren, g_fnt_sm, g_menu[order_menu].name, bx + 16, by + bubble_h - 13, nameColor);
		}
	}
}

void draw_barista_slots(SDL_Renderer* ren, Game* g) {
	SDL_Rect counterLine = { 0, 370, SCREEN_W, 10 };
	SDL_SetRenderDrawColor(ren, 115, 80, 60, 255);
	SDL_RenderFillRect(ren, &counterLine);
	SDL_Rect machineBg = { 50, 360, 250, 160 };
	SDL_SetRenderDrawColor(ren, 127, 140, 141, 255);
	SDL_RenderFillRect(ren, &machineBg);
	SDL_SetRenderDrawColor(ren, 189, 195, 199, 255);
	SDL_RenderDrawRect(ren, &machineBg);
	SDL_Rect machineHeader = { 50, 360, 250, 25 };
	SDL_SetRenderDrawColor(ren, 44, 62, 80, 255);
	SDL_RenderFillRect(ren, &machineHeader);
	SDL_Color white = { 255, 255, 255, 255 };
	draw_text(ren, g_fnt_sm, "☕ CUTE ESPRESSO MACHINE", 65, 365, white);

	for (int i = 0; i < g->slot_count; i++) {
		int sx = 70 + i * 80; int sy = 400; SDL_Rect cupSpot = { sx, sy, 60, 80 };
		if (g->sel_slot == i) {
			SDL_Rect selSlotOutline = { sx - 4, sy - 4, 68, 88 };
			SDL_SetRenderDrawColor(ren, 254, 202, 87, 255);
			SDL_RenderDrawRect(ren, &selSlotOutline);
		}
		SDL_Rect nozzle = { sx + 22, sy - 12, 16, 12 };
		SDL_SetRenderDrawColor(ren, 52, 73, 94, 255);
		SDL_RenderFillRect(ren, &nozzle);

		SlotState state = g->slots[i].state;
		if (state == SLOT_EMPTY) {
			SDL_SetRenderDrawColor(ren, 90, 105, 120, 180);
			SDL_RenderFillRect(ren, &cupSpot);
			SDL_Color textGray = { 200, 214, 229, 255 };
			char numStr[128];
			sprintf_s(numStr, sizeof(numStr), "#%d 빈슬롯", i + 1);
			draw_text(ren, g_fnt_sm, numStr, sx + 5, sy + 32, textGray);
		}
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
			int bar_max_w = 50; float brew_ratio = (float)g->slots[i].elapsed_ms / g->slots[i].required_ms;
			if (brew_ratio > 1.0f) brew_ratio = 1.0f;
			int bar_curr_w = (int)(bar_max_w * brew_ratio);
			SDL_Rect progressBg = { sx + 5, sy + 5, bar_max_w, 6 };
			SDL_SetRenderDrawColor(ren, 87, 101, 116, 255);
			SDL_RenderFillRect(ren, &progressBg);
			SDL_Rect progressFill = { sx + 5, sy + 5, bar_curr_w, 6 };
			SDL_SetRenderDrawColor(ren, 9, 132, 227, 255);
			SDL_RenderFillRect(ren, &progressFill);
		}
		else if (state == SLOT_DONE) {
			SDL_SetRenderDrawColor(ren, 108, 92, 231, 100);
			SDL_RenderFillRect(ren, &cupSpot);
			draw_procedural_cup(ren, sx, sy + 20, 60, 60, g->slots[i].menu, true);
			Uint32 tick = SDL_GetTicks(); int steam_offset = (tick / 200) % 3;
			SDL_SetRenderDrawColor(ren, 255, 255, 255, 180);
			SDL_RenderDrawLine(ren, sx + 20 + steam_offset, sy + 15, sx + 20 + steam_offset, sy + 5);
			SDL_Color textGreen = { 76, 209, 55, 255 };
			draw_text(ren, g_fnt_sm, "★완료★", sx + 8, sy + 2, textGreen);
		}
	}

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
		for (int i = 0; i < 6; i++) {
			int bx = mx_start + 10 + i * 56; int by = my_start + 26; SDL_Rect btnRect = { bx, by, 45, 45 };
			if (g_menu[i].unlocked) {
				SDL_SetRenderDrawColor(ren, 115, 96, 83, 255);
				SDL_RenderFillRect(ren, &btnRect);
				if (g_tex_menus[i]) SDL_RenderCopy(ren, g_tex_menus[i], NULL, &btnRect);
				else draw_procedural_cup(ren, bx, by, 45, 45, (MenuID)i, true);
				SDL_Color wt = { 255, 255, 255, 255 };
				draw_text(ren, g_fnt_sm, hotkeys[i], bx + 18, by + 12, wt);
			}
			else {
				SDL_SetRenderDrawColor(ren, 50, 40, 35, 255);
				SDL_RenderFillRect(ren, &btnRect);
				SDL_Color lockGray = { 150, 140, 135, 255 };
				draw_text(ren, g_fnt_sm, "🔒", bx + 14, by + 12, lockGray);
			}
		}
	}
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