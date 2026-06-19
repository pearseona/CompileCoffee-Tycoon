#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "shop.h"
#include "game.h"

/* 상점 아이템 업그레이드, 재료 구매, 메뉴 레시피 해금 연산 통합 */
void shop_buy_upgrade(Game* g, int upg_idx) {
	// 6개 슬롯 전체 인덱스 방어 (0~5)
	if (upg_idx < 0 || upg_idx >= 6)
		return;

	Upgrade* u = &g->upg[upg_idx];

	// [1] 재료 도매 구매 분기 처리 (2, 3번)
	if (upg_idx == 2 || upg_idx == 3) {
		int target_ing = (upg_idx == 2) ? ING_BEAN : ING_MILK;
		int bulk_count = 10;

		if (g->balance < u->base_cost) {
			log_push(g, "잔액이 부족하여 재료를 도매가로 구매할 수 없습니다!");
			return;
		}

		g->balance -= u->base_cost;
		g->stock[target_ing] += bulk_count;

		char shop_msg[80];
		sprintf_s(shop_msg, sizeof(shop_msg), "📦 %s %d개 도매 구매 완료! (창고 입고)", (target_ing == ING_BEAN) ? "원두" : "우유", bulk_count);
		log_push(g, shop_msg);
		return;
	}

	// [2] 신메뉴 레시피 해금 분기 처리 (4, 5번)
	if (upg_idx == 4 || upg_idx == 5) {
		int target_menu = (upg_idx == 4) ? MENU_VANILLA_LATTE : MENU_COLD_BREW;

		if (g_menu[target_menu].unlocked) {
			log_push(g, "이미 레시피를 전격 해금한 베스트 메뉴입니다!");
			return;
		}

		if (g->balance < u->base_cost) {
			log_push(g, "잔액이 부족하여 메뉴를 해금할 수 없습니다!");
			return;
		}

		g->balance -= u->base_cost;
		u->level = 1;
		g_menu[target_menu].unlocked = 1; // ⭐ 핵심: 전역 메뉴 테이블 데이터 락 해제!

		char shop_msg[80];
		sprintf_s(shop_msg, sizeof(shop_msg), "☕ %s 레시피 해금 완료! 이제 손님이 주문합니다.", g_menu[target_menu].name);
		log_push(g, shop_msg);
		printf("[SHOP] 메뉴 해금 성공: %s (남은 잔액: %d원)\n", u->name, g->balance);
		return;
	}

	// [3] 기존 업그레이드 아이템 로직 (슬롯 확장, 머신 속도)
	if (u->level >= u->max_level) {
		log_push(g, "이미 최고 레벨까지 업그레이드된 항목입니다!");
		return;
	}

	if (g->balance < u->base_cost) {
		log_push(g, "잔액이 부족하여 업그레이드할 수 없습니다!");
		return;
	}

	g->balance -= u->base_cost;
	u->level++;

	if (upg_idx == 0) {
		g->slot_count = u->level;
		log_push(g, "제조 슬롯이 성공적으로 확장되었습니다!");
	}
	else if (upg_idx == 1) {
		log_push(g, "커피 머신 속도가 향상되었습니다!");
	}
	printf("[SHOP] 구매 성공: %s Lv.%d (남은 잔액: %d원)\n", u->name, u->level, g->balance);
}

/* 상점 정비를 마치고 다음 날 영업 진행 */
void shop_next_day(Game* g) {
	g->day++;
	g->shop_page = 0; // 상점 창 페이지 디폴트 리셋 초기화

	printf("\n========================================\n");
	printf(" 제 %d일차 아침이 밝았습니다! 영업을 준비하세요.\n", g->day);
	printf("========================================\n\n");

	game_start_day(g);
}