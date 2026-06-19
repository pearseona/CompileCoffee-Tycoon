#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "shop.h"
#include "game.h"

/* 상점 아이템 업그레이드 및 재료 구매 연산 */
void shop_buy_upgrade(Game* g, int upg_idx) {
	// 4개 슬롯으로 범위 확장 (0:슬롯, 1:속도, 2:원두 도매, 3:우유 도매)
	if (upg_idx < 0 || upg_idx >= 4)
		return;

	Upgrade* u = &g->upg[upg_idx];

	// [대안 B] 재료 구매 분기 처리 (upg_idx 2, 3은 일회성 소모품 구매)
	if (upg_idx == 2 || upg_idx == 3) {
		int target_ing = (upg_idx == 2) ? ING_BEAN : ING_MILK;
		int bulk_count = 10; // 한 번에 10개씩 묶음 판매

		if (g->balance < u->base_cost) {
			log_push(g, "잔액이 부족하여 재료를 도매가로 구매할 수 없습니다!");
			return;
		}

		g->balance -= u->base_cost;
		g->stock[target_ing] += bulk_count;

		char shop_msg[80];
		sprintf_s(shop_msg, sizeof(shop_msg), "📦 %s %d개 도매 구매 완료! (창고 입고)", (target_ing == ING_BEAN) ? "원두" : "우유", bulk_count);
		log_push(g, shop_msg);
		printf("[SHOP] 재료 소급: %s +%d개 (남은 잔액: %d원)\n", u->name, bulk_count, g->balance);
		return; // 일반 업그레이드 스펙 반영 스킵
	}

	// ================= 기존 업그레이드 아이템 로직 (슬롯 확장, 머신 속도) =================
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
	g->day++; // 날짜 카운트 1 증가

	printf("\n========================================\n");
	printf(" 제 %d일차 아침이 밝았습니다! 영업을 준비하세요.\n", g->day);
	printf("========================================\n\n");

	game_start_day(g); // 새로운 날의 변수와 타이머로 게임 진행
}