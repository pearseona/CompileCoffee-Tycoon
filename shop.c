#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "shop.h"
#include "game.h"

/* 상점 아이템 업그레이드, 재료 구매, 메뉴 레시피 해금 연산 통합 대확장 */
void shop_buy_upgrade(Game* g, int upg_idx) {
	// 🎯 [대확장]: 아이템 인덱스를 0~9번(총 10개 항목)으로 인덱스 가드 확장 완료!
	if (upg_idx < 0 || upg_idx >= 10)
		return;

	Upgrade* u = &g->upg[upg_idx];

	// [1] 재료 도매 구매 분기 처리 (2:원두, 3:우유, 6:시럽, 7:크림, 8:얼음)
	if (upg_idx == 2 || upg_idx == 3 || upg_idx == 6 || upg_idx == 7 || upg_idx == 8) {
		int target_ing = ING_BEAN;
		const char* ing_name = "원두";
		int bulk_cost = 1500;

		if (upg_idx == 2) { target_ing = ING_BEAN; ing_name = "원두"; bulk_cost = 1500; }
		else if (upg_idx == 3) { target_ing = ING_MILK; ing_name = "우유"; bulk_cost = 1000; }
		else if (upg_idx == 6) { target_ing = ING_SYRUP; ing_name = "시럽"; bulk_cost = 1200; }
		else if (upg_idx == 7) { target_ing = ING_CREAM; ing_name = "크림"; bulk_cost = 1400; }
		else if (upg_idx == 8) { target_ing = ING_ICE;   ing_name = "얼음"; bulk_cost = 800; }

		int bulk_count = 10;

		if (g->balance < bulk_cost) {
			log_push(g, "잔액이 부족하여 재료를 도매가로 구매할 수 없습니다!");
			return;
		}

		g->balance -= bulk_cost;
		g->stock[target_ing] += bulk_count;

		char shop_msg[80];
		sprintf_s(shop_msg, sizeof(shop_msg), "📦 %s %d개 도매 구매 완료! (창고 입고)", ing_name, bulk_count);
		log_push(g, shop_msg);
		return;
	}

	// [2] 신메뉴 레시피 해금 분기 처리 (4:바닐라라떼, 5:콜드브루, 9:카라멜마끼아또)
	if (upg_idx == 4 || upg_idx == 5 || upg_idx == 9) {
		int target_menu = MENU_VANILLA_LATTE;
		int research_cost = 6000;

		if (upg_idx == 4) { target_menu = MENU_VANILLA_LATTE; research_cost = 6000; }
		else if (upg_idx == 5) { target_menu = MENU_COLD_BREW; research_cost = 9000; }
		else if (upg_idx == 9) { target_menu = MENU_CARAMEL_MAC; research_cost = 12000; }

		if (g_menu[target_menu].unlocked) {
			log_push(g, "이미 레시피를 전격 해금한 베스트 메뉴입니다!");
			return;
		}

		if (g->balance < research_cost) {
			log_push(g, "잔액이 부족하여 신메뉴 레시피를 연구할 수 없습니다!");
			return;
		}

		g->balance -= research_cost;
		u->level = 1;
		g_menu[target_menu].unlocked = 1;

		char shop_msg[256];
		sprintf_s(shop_msg, sizeof(shop_msg), "☕ %s 레시피 해금 완료! 이제 손님이 주문합니다.", g_menu[target_menu].name);
		log_push(g, shop_msg);
		printf("[SHOP] 메뉴 해금 성공: %s (남은 잔액: %d원)\n", g_menu[target_menu].name, g->balance);
		return;
	}

	// [3] 기존 업그레이드 장비 항목 로직 (0:슬롯 확장, 1:머신 속도)
	int upgrade_cost = (upg_idx == 0) ? 3000 : 25000;
	if (u->level >= u->max_level) {
		log_push(g, "이미 최고 레벨까지 업그레이드된 항목입니다!");
		return;
	}

	if (g->balance < upgrade_cost) {
		log_push(g, "잔액이 부족하여 업그레이드할 수 없습니다!");
		return;
	}

	g->balance -= upgrade_cost;
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
	g->shop_page = 0;

	printf("\n========================================\n");
	printf(" 제 %d일차 아침이 밝았습니다! 영업을 준비하세요.\n", g->day);
	printf("========================================\n\n");

	game_start_day(g);
}