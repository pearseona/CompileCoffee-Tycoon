#define _CRT_SECURE_NO_WARNINGS
#include "../render.h"
#include "../customer.h"

extern SDL_Texture* g_tex_customers[3];
extern SDL_Texture* g_tex_barista;
extern SDL_Texture* g_tex_station;
extern SDL_Texture* g_tex_background; 

extern TTF_Font* g_fnt_sm;

/*  배경 이미지 출력 및 간판 변경 */
void draw_game_background(SDL_Renderer* ren) {
	if (g_tex_background) {

		SDL_Rect bgRect = { 0, 0, SCREEN_W, SCREEN_H };
		SDL_RenderCopy(ren, g_tex_background, NULL, &bgRect);

		SDL_Rect coverRect = { 330, 485, 140, 38 };
		SDL_SetRenderDrawColor(ren, 85, 54, 40, 255); 
		SDL_RenderFillRect(ren, &coverRect);

		SDL_Color gold = { 241, 196, 15, 255 };
		draw_text(ren, g_fnt_sm, "COMPILE COFFEE", 345, 495, gold);
	}
}

/* ==== 손님 대기 큐 영역 ==== */
void draw_customer_queue(SDL_Renderer* ren, Game* g) {

	int start_x = 45;   
	int start_y = 110;  
	int card_w = 85;    
	int card_h = 135;  
	int spacing = 15;   

	for (int i = 0; i < MAX_QUEUE; i++) {

		int current_y = start_y + i * (card_h + spacing);

		if (g->queue[i].active == 1) {
			double patience_ratio = (double)g->queue[i].patience_ms / g->queue[i].patience_max;
			patience_ratio = (patience_ratio < 0.0) ? 0.0 : (patience_ratio > 1.0 ? 1.0 : patience_ratio);

			SDL_Rect customerCard = { start_x, current_y, card_w, card_h };

			int c_type = (int)g->queue[i].type;

			if (c_type >= 0 && c_type < 3 && g_tex_customers[c_type]) {
				SDL_RenderCopy(ren, g_tex_customers[c_type], NULL, &customerCard);
			}
			else {
				if (g->queue[i].type == CUST_WORKER) SDL_SetRenderDrawColor(ren, 100, 149, 237, 255);
				else if (g->queue[i].type == CUST_FOODIE) SDL_SetRenderDrawColor(ren, 218, 165, 32, 255);
				else if (g->queue[i].type == CUST_STUDENT) SDL_SetRenderDrawColor(ren, 143, 188, 143, 255);
				SDL_RenderFillRect(ren, &customerCard);
			}

			/* 주문 말풍선 박스 */
			SDL_Rect orderBox = { start_x + card_w + 8, current_y + 25, 65, 25 };
			if (g->queue[i].order == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);
			else if (g->queue[i].order == MENU_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
			else if (g->queue[i].order == MENU_VANILLA_LATTE) SDL_SetRenderDrawColor(ren, 255, 239, 186, 255);
			else if (g->queue[i].order == MENU_COLD_BREW) SDL_SetRenderDrawColor(ren, 62, 39, 35, 255);
			else if (g->queue[i].order == MENU_CARAMEL_MAC) SDL_SetRenderDrawColor(ren, 216, 112, 147, 255);
			else if (g->queue[i].order == MENU_ESPRESSO) SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);
			SDL_RenderFillRect(ren, &orderBox);

			/* 인내심 감정 */
			SDL_Rect emojiBox = { start_x + card_w + 78, current_y + 31, 12, 12 };
			if (patience_ratio > 0.5) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
			else if (patience_ratio > 0.25) SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
			else SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
			SDL_RenderFillRect(ren, &emojiBox);

			/* 인내심 게이지 바 */
			int gauge_max_w = card_w - 10;
			int gauge_h = 7;
			int gauge_x = start_x + 5;
			int gauge_y = current_y - 12;
			int current_gauge_w = (int)(gauge_max_w * patience_ratio);

			SDL_Rect bgGauge = { gauge_x, gauge_y, gauge_max_w, gauge_h };
			SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
			SDL_RenderFillRect(ren, &bgGauge);

			SDL_Rect fillGauge = { gauge_x, gauge_y, current_gauge_w, gauge_h };
			if (patience_ratio > 0.5) SDL_SetRenderDrawColor(ren, 46, 204, 113, 255);
			else if (patience_ratio > 0.25) SDL_SetRenderDrawColor(ren, 241, 196, 15, 255);
			else SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);
			SDL_RenderFillRect(ren, &fillGauge);
		}
	}
}

/* ===== 바리스타 제조 워크스테이션 ===== */
void draw_barista_slots(SDL_Renderer* ren, Game* g) {

	int slot_start_x = 240; 
	int slot_start_y = 310; 
	int slot_w = 120;
	int slot_h = 110;    
	int slot_spacing = 15;

	for (int i = 0; i < g->slot_count; i++) {
		int current_slot_x = slot_start_x + i * (slot_w + slot_spacing);
		SDL_Rect slotRect = { current_slot_x, slot_start_y, slot_w, slot_h };

		if (g->slots[i].state == SLOT_EMPTY) {
			if (g_tex_station) {
				SDL_RenderCopy(ren, g_tex_station, NULL, &slotRect);
			}
			else {
				SDL_SetRenderDrawColor(ren, 45, 45, 48, 255);
				SDL_RenderFillRect(ren, &slotRect);
				SDL_SetRenderDrawColor(ren, 70, 70, 75, 255);
				SDL_RenderDrawRect(ren, &slotRect);
			}
		}
		else if (g->slots[i].state == SLOT_BREWING) {

			SDL_SetRenderDrawColor(ren, 141, 110, 99, 180); 
			SDL_RenderFillRect(ren, &slotRect);

			SDL_Rect miniMenuBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 22 };
			MenuID mid = g->slots[i].menu;
			if (mid == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);
			else if (mid == MENU_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
			else SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);
			SDL_RenderFillRect(ren, &miniMenuBox);

			// 실시간 제조 진행 바
			int brew_bar_max_w = slot_w - 30;
			int brew_bar_h = 12;
			int brew_bar_x = current_slot_x + 15;
			int brew_bar_y = slot_start_y + slot_h - 25;

			float brew_ratio = (float)g->slots[i].elapsed_ms / g->slots[i].required_ms;
			if (brew_ratio > 1.0f) brew_ratio = 1.0f;
			int brew_bar_current_w = (int)(brew_bar_max_w * brew_ratio);

			SDL_Rect bgBrewBar = { brew_bar_x, brew_bar_y, brew_bar_max_w, brew_bar_h };
			SDL_SetRenderDrawColor(ren, 60, 60, 60, 255);
			SDL_RenderFillRect(ren, &bgBrewBar);

			SDL_Rect fillBrewBar = { brew_bar_x, brew_bar_y, brew_bar_current_w, brew_bar_h };
			SDL_SetRenderDrawColor(ren, 52, 152, 219, 255);
			SDL_RenderFillRect(ren, &fillBrewBar);
		}
		else if (g->slots[i].state == SLOT_DONE) {

			SDL_SetRenderDrawColor(ren, 38, 166, 154, 200);
			SDL_RenderFillRect(ren, &slotRect);

			SDL_Rect alertBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 40 };
			SDL_SetRenderDrawColor(ren, 240, 240, 240, 255);
			SDL_RenderFillRect(ren, &alertBox);
		}
	}
}