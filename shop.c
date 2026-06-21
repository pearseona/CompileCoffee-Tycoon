#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "shop.h"
#include "game.h"

/* 상점 아이템 구매 & 업그레이드 */
void shop_buy_upgrade(Game* g, int upg_idx) {

	// 인덱스 검증
	if (upg_idx < 0 || upg_idx >= 10)
		return;

	Upgrade* u = &g->upg[upg_idx];

	// 원자재 구매 
	if (upg_idx == 2 || upg_idx == 3 || upg_idx == 6 || upg_idx == 7 || upg_idx == 8) {
		int target_ing = ING_BEAN;
		const char* ing_name = "원두";
		int bulk_cost = 1500;

		// 각 재료별 인덱스, 명칭, 가격 매핑
		if (upg_idx == 2) { target_ing = ING_BEAN; ing_name = "원두"; bulk_cost = 1500; }
		else if (upg_idx == 3) { target_ing = ING_MILK; ing_name = "우유"; bulk_cost = 1000; }
		else if (upg_idx == 6) { target_ing = ING_SYRUP; ing_name = "시럽"; bulk_cost = 1200; }
		else if (upg_idx == 7) { target_ing = ING_CREAM; ing_name = "크림"; bulk_cost = 1400; }
		else if (upg_idx == 8) { target_ing = ING_ICE;   ing_name = "얼음"; bulk_cost = 800; }

		int bulk_count = 10; // 구매 수량

		// 잔액 확인 및 구매
		if (g->balance < bulk_cost) {
			log_push(g, "잔액이 부족하여 재료를 도매가로 구매할 수 없습니다!");
			return;
		}

		g->balance -= bulk_cost;
		g->stock[target_ing] += bulk_count;

		char shop_msg[80];
		sprintf_s(shop_msg, sizeof(shop_msg), "📦 %s %d개 구매 완료! (창고 입고)", ing_name, bulk_count);
		log_push(g, shop_msg);
		return;
	}

	// 메뉴 레시피 해금 (바닐라라떼, 콜드브루, 카라멜마끼아또)
	if (upg_idx == 4 || upg_idx == 5 || upg_idx == 9) {
		int target_menu = MENU_VANILLA_LATTE;
		int research_cost = 6000;

		if (upg_idx == 4) { target_menu = MENU_VANILLA_LATTE; research_cost = 6000; }
		else if (upg_idx == 5) { target_menu = MENU_COLD_BREW; research_cost = 9000; }
		else if (upg_idx == 9) { target_menu = MENU_CARAMEL_MAC; research_cost = 12000; }

		// 중복 해금 방지 및 잔액 검증
		if (g_menu[target_menu].unlocked) {
			log_push(g, "이미 레시피를 해금한 메뉴입니다!");
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

	// 장비 및 시설 업그레이드

	// 제조 슬롯 확장 및 머신 작업 속도 강화
	int upgrade_cost = (upg_idx == 0) ? 3000 : 25000;

	// 최대 레벨 도달 검사 및 잔액 검증
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

	// 슬롯 확장 시 제조 가능한 전체 슬롯 카운팅 갱신
	if (upg_idx == 0) {
		g->slot_count = u->level;
		log_push(g, "제조 슬롯이 성공적으로 확장되었습니다!");
	}
	else if (upg_idx == 1) {
		log_push(g, "커피 머신 속도가 향상되었습니다!");
	}
	printf("[SHOP] 구매 성공: %s Lv.%d (남은 잔액: %d원)\n", u->name, u->level, g->balance);
}

/* 상점 정비 후 다음 날 영업 진행 */
void shop_next_day(Game* g) {
	g->day++;
	g->shop_page = 0;

	printf("\n========================================\n");
	printf(" 제 %d일차 아침이 밝았습니다! 영업을 준비하세요.\n", g->day);
	printf("========================================\n\n");

	game_start_day(g); 
}