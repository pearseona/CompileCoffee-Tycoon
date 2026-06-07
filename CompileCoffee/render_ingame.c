#define _CRT_SECURE_NO_WARNINGS
#include "../render.h"
#include "../customer.h"

/* ===== 손님 대기 큐 영역 렌더링 ===== */
void draw_customer_queue(SDL_Renderer* ren, Game* g) {

	// 화면 중앙에 최대 8명의 손님 자리를 가로로 배치
	int start_x = 40;
	int start_y = 120;
	int card_w = 100; // 손님 캐릭터 및 주문이 보일 사각형 카드 너비
	int card_h = 160; // 카드 높이
	int spacing = 12; // 손님 사이의 간격

	for (int i = 0; i < MAX_QUEUE; i++) {
		int current_x = start_x + i * (card_w + spacing);

		// 손님이 실시간으로 서 있는 슬롯인 경우에만 렌더링
		if (g->queue[i].active == 1) {

			// 인내심 비율 계산
			double patience_ratio = (double)g->queue[i].patience_ms / g->queue[i].patience_max;
			patience_ratio = (patience_ratio < 0.0) ? 0.0 : (patience_ratio > 1.0 ? 1.0 : patience_ratio);

			/*  손님 카드 배경 */
			SDL_Rect customerCard = { current_x, start_y, card_w, card_h };

			if (g->queue[i].type == CUST_WORKER) {
				SDL_SetRenderDrawColor(ren, 100, 149, 237, 255); // 직장인: 블루
			}
			else if (g->queue[i].type == CUST_FOODIE) {
				SDL_SetRenderDrawColor(ren, 218, 165, 32, 255); // 미식가: 골드
			}
			else if (g->queue[i].type == CUST_STUDENT) {
				SDL_SetRenderDrawColor(ren, 143, 188, 143, 255); // 학생: 다크 그린
			}
			SDL_RenderFillRect(ren, &customerCard);


			/* 주문한 음료 정보 표시 */
			SDL_Rect orderBox = { current_x + 10, start_y + 10, card_w - 20, 30 };

			if (g->queue[i].order == MENU_AMERICANO) {
				SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);   // 아메리카노: 갈색
			}
			else if (g->queue[i].order == MENU_LATTE) {
				SDL_SetRenderDrawColor(ren, 245, 222, 179, 255); // 카페라떼: 베이지색
			}
			else if (g->queue[i].order == MENU_VANILLA_LATTE) {
				SDL_SetRenderDrawColor(ren, 255, 239, 186, 255); // 바닐라라떼: 밝은 노란빛
			}
			else if (g->queue[i].order == MENU_COLD_BREW) {
				SDL_SetRenderDrawColor(ren, 62, 39, 35, 255);    // 콜드브루: 흑갈색
			}
			else if (g->queue[i].order == MENU_CARAMEL_MAC) {
				SDL_SetRenderDrawColor(ren, 216, 112, 147, 255); // 카라멜 마키아토: 핑크빛 갈색
			}
			else if (g->queue[i].order == MENU_ESPRESSO) {
				SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);     // 에스프레소: 진한 검은갈색
			}

			SDL_RenderFillRect(ren, &orderBox);

			/* 인내심 비율에 따른 만족도 상태 (추후 이모지로 대체) */
			SDL_Rect emojiBox = { current_x + card_w - 25, start_y + 45, 15, 15 };

			if (patience_ratio > 0.5) {
				SDL_SetRenderDrawColor(ren, 0, 255, 0, 255); // 초록 점: 좋음 😊
			}
			else if (patience_ratio > 0.25) {
				SDL_SetRenderDrawColor(ren, 255, 255, 0, 255); // 노란 점: 보통 😐
			}
			else {
				SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);   // 빨간 점: 위험 😡) 
			}

			SDL_RenderFillRect(ren, &emojiBox);

			/* 머리 위 실시간 인내심 3색 게이지 바 */
			int gauge_max_w = card_w - 20; // 게이지 전체 너비 
			int gauge_h = 10;
			int gauge_x = current_x + 10;
			int gauge_y = start_y + card_h - 25;

			int current_gauge_w = (int)(gauge_max_w * patience_ratio);

			// 게이지바 어두운 배경들
			SDL_Rect bgGauge = { gauge_x, gauge_y, gauge_max_w, gauge_h };
			SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
			SDL_RenderFillRect(ren, &bgGauge);

			// 실시간 알맹이 게이지
			SDL_Rect fillGauge = { gauge_x, gauge_y, current_gauge_w, gauge_h };

			if (patience_ratio > 0.5) {
				SDL_SetRenderDrawColor(ren, 46, 204, 113, 255);  // 50% 이상: 안전 초록색
			}
			else if (patience_ratio > 0.25) {
				SDL_SetRenderDrawColor(ren, 241, 196, 15, 255); // 25%~50%: 경고 노란색
			}
			else {
				SDL_SetRenderDrawColor(ren, 231, 76, 60, 255);  // 25% 이하: 위험 빨간색
			}
			SDL_RenderFillRect(ren, &fillGauge);
		}
		else {
			// 손님이 없는 빈 슬롯
			SDL_Rect emptyCard = { current_x, start_y, card_w, card_h };
			SDL_SetRenderDrawColor(ren, 45, 45, 50, 255);
			SDL_RenderDrawRect(ren, &emptyCard);
		}
	}

}

/* === 바리스타 제조 워크스테이션(슬롯) 렌더링 */
void draw_barista_slots(SDL_Renderer* ren, Game* g) {

	int slot_start_x = 40;
	int slot_start_y = 330; // 손님 레이어 및 공간에 배치
	int slot_w = 140; // 제조 슬롯 상자 너비
	int slot_h = 130; // 제조 슬롯 상자 너비
	int slot_spacing = 20;

	for (int i = 0; i < g->slot_count; i++) {
		int current_slot_x = slot_start_x + i * (slot_w + slot_spacing);
		SDL_Rect slotRect = { current_slot_x, slot_start_y, slot_w, slot_h };

		// 슬롯 상태별 테두리 및 내부 배경 색상 분기
		if (g->slots[i].state == SLOT_EMPTY) {
			// 빈 대기 상태 워크스테이션 (어두운 회색)
			SDL_SetRenderDrawColor(ren, 45, 45, 48, 255);
			SDL_RenderFillRect(ren, &slotRect);
			SDL_SetRenderDrawColor(ren, 70, 70, 75, 255);
			SDL_RenderDrawRect(ren, &slotRect);
		}
		else if (g->slots[i].state == SLOT_BREWING) {
			// 음료 추출
			SDL_SetRenderDrawColor(ren, 141, 110, 99, 255);
			SDL_RenderFillRect(ren, &slotRect);

			// 제조 중인 메뉴 (고유 색으로 메뉴판 박스 표시)
			SDL_Rect miniMenuBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 25 };
			MenuID mid = g->slots[i].menu;
			if (mid == MENU_AMERICANO) SDL_SetRenderDrawColor(ren, 121, 85, 72, 255);
			else if (mid == MENU_LATTE) SDL_SetRenderDrawColor(ren, 245, 222, 179, 255);
			else SDL_SetRenderDrawColor(ren, 38, 24, 22, 255);
			SDL_RenderFillRect(ren, &miniMenuBox);

			// 실시간 바리스타 작업 게이지 바 누적 렌더링
			int brew_bar_max_w = slot_w - 30;
			int brew_bar_h = 15;
			int brew_bar_x = current_slot_x + 15;
			int brew_bar_y = slot_start_y + slot_h - 35;

			// 누적 비율 계산
			float brew_ratio = (float)g->slots[i].elapsed_ms / g->slots[i].required_ms;
			if (brew_ratio > 1.0f) brew_ratio = 1.0f;
			int brew_bar_current_w = (int)(brew_bar_max_w * brew_ratio);

			// 게이지 배경
			SDL_Rect bgBrewBar = { brew_bar_x, brew_bar_y, brew_bar_max_w, brew_bar_h };
			SDL_SetRenderDrawColor(ren, 60, 60, 60, 255);
			SDL_RenderFillRect(ren, &bgBrewBar);

			// 차오르는 게이지
			SDL_Rect fillBrewBar = { brew_bar_x, brew_bar_y, brew_bar_current_w, brew_bar_h };
			SDL_SetRenderDrawColor(ren, 52, 152, 219, 255); // 블루
			SDL_RenderFillRect(ren, &fillBrewBar);

			if (g->slots[i].state == SLOT_DONE) {
				// 제조 완료 서빙 대기
				SDL_SetRenderDrawColor(ren, 38, 166, 154, 255); // 민트/네온 그린
				SDL_RenderFillRect(ren, &slotRect);

				// 상단에 완성 알림 표시
				SDL_Rect alertBox = { current_slot_x + 15, slot_start_y + 15, slot_w - 30, 45 };
				SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); // 흰색
				SDL_RenderFillRect(ren, &alertBox);
			}
		}

	}
}


