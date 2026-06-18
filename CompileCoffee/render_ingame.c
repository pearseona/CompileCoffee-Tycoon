#define _CRT_SECURE_NO_WARNINGS
#include "../render.h"
#include "../customer.h"

// render.c의 그래픽 텍스처 외부 참조
extern SDL_Texture* g_tex_customers[3];
extern SDL_Texture* g_tex_barista;
extern SDL_Texture* g_tex_station;

/* ===== 손님 대기 큐 영역 렌더링 ===== */
void draw_customer_queue(SDL_Renderer* ren, Game* g) {
	int start_x = 40;
	int start_y = 120;
	int card_w = 100; // 카드 너비
	int card_h = 160; // 카드 높이
	int spacing = 12; // 간격

	for (int i = 0; i < MAX_QUEUE; i++) {
		int current_x = start_x + i * (card_w + spacing);

		if (g->queue[i].active == 1) {
			// 인내심 비율 계산
			double patience_ratio = (double)g->queue[i].patience_ms / g->queue[i].patience_max;
			patience_ratio = (patience_ratio < 0.0) ? 0.0 : (patience_ratio > 1.0 ? 1.0 : patience_ratio);

			SDL_Rect customerCard = { current_x, start_y, card_w, card_h };

			int c_type = (int)g->queue[i].type;

			// 픽셀 이미지로 렌더링
			if (c_type >= 0 && c_type < 3 && g_tex_customers[c_type]) {
				SDL_RenderCopy(ren, g_tex_customers[c_type], NULL, &customerCard);
			}
			else {
				if (g->queue[i].type == CUST_WORKER) SDL_SetRenderDrawColor(ren, 100, 149, 237, 255);
				else if (g->queue[i].type == CUST_FOODIE) SDL_SetRenderDrawColor(ren, 218, 165, 32, 255);
				else if (g->queue[i].type == CUST_STUDENT) SDL_SetRenderDrawColor(ren, 143, 188, 143, 255);
				SDL_RenderFillRect(ren, &customerCard);
			}

			/* 주문한 음료 정보 표시 */
			SDL_Rect orderBox = { current_x + 10, start_y - 25, card_w - 20, 25 };
			if (g->queue[i].order == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);
			else if (g->queue[i].order == MENU_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
			else if (g->queue[i].order == MENU_VANILLA_LATTE) SDL_SetRenderDrawColor(ren, 255, 239, 186, 255);
			else if (g->queue[i].order == MENU_COLD_BREW) SDL_SetRenderDrawColor(ren, 62, 39, 35, 255);
			else if (g->queue[i].order == MENU_CARAMEL_MAC) SDL_SetRenderDrawColor(ren, 216, 112, 147, 255);
			else if (g->queue[i].order == MENU_ESPRESSO) SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);
			SDL_RenderFillRect(ren, &orderBox);

			/* 인내심 서클 인디케이터 */
			SDL_Rect emojiBox = { current_x + card_w - 22, start_y - 20, 12, 12 };
			if (patience_ratio > 0.5) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
			else if (patience_ratio > 0.25) SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
			else SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
			SDL_RenderFillRect(ren, &emojiBox);

			/* 머리 위 실시간 인내심 3색 게이지 바 */
			int gauge_max_w = card_w - 20;
			int gauge_h = 8;
			int gauge_x = current_x + 10;
			int gauge_y = start_y + card_h + 5;
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
		else {
			// 빈 슬롯 테두리 보드 처리
			SDL_Rect emptyCard = { current_x, start_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, 45, 45, 50, 255);
			SDL_RenderDrawRect(ren, &emptyCard);
		}
	}
}

/* ==== 바리스타 제조 워크스테이션(슬롯) 렌더링 ==== */
void draw_barista_slots(SDL_Renderer* ren, Game* g) {
	int slot_start_x = 40;
	int slot_start_y = 330;
	int slot_w = 140;
	int slot_h = 130;
	int slot_spacing = 20;

	for (int i = 0; i < g->slot_count; i++) {
		int current_slot_x = slot_start_x + i * (slot_w + slot_spacing);
		SDL_Rect slotRect = { current_slot_x, slot_start_y, slot_w, slot_h };

		if (g->slots[i].state == SLOT_EMPTY) {
			// 빈 기계 자리에 에스프레소 머신 도트 이미지 
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
			// 음료 추출 중 
			SDL_SetRenderDrawColor(ren, 141, 110, 99, 255);
			SDL_RenderFillRect(ren, &slotRect);

			SDL_Rect miniMenuBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 25 };
			MenuID mid = g->slots[i].menu;
			if (mid == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);
			else if (mid == MENU_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
			else SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);
			SDL_RenderFillRect(ren, &miniMenuBox);

			// 실시간 게이지 연산
			int brew_bar_max_w = slot_w - 30;
			int brew_bar_h = 15;
			int brew_bar_x = current_slot_x + 15;
			int brew_bar_y = slot_start_y + slot_h - 35;

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

			SDL_SetRenderDrawColor(ren, 38, 166, 154, 255); 
			SDL_RenderFillRect(ren, &slotRect);

			SDL_Rect alertBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 45 };
			SDL_SetRenderDrawColor(ren, 240, 240, 240, 255);
			SDL_RenderFillRect(ren, &alertBox);
		}
	}
}