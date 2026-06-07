#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "shop.h"
#include "game.h"

/* 상점 아이템 업그레이드 구매 연산 */
void shop_buy_upgrade(Game* g, int upg_idx) {
	if (upg_idx < 0 || upg_idx >= 2)
		return;

	Upgrade* u = &g->upg[upg_idx];

	// 예외 처리 1: 이미 마스터 레벨인 경우
	if (u->level >= u->max_level) {
		log_push(g, "이미 최고 레벨까지 업그레이드된 항목입니다!");
		printf("[SHOP] 구매 실패: 최고 레벨 도달 (%s)\n", u->name);
		return;
	}

	// 예외 처리 2: 소지한 잔액이 부족한 경우
	if (g->balance < u->base_cost) {
		log_push(g, "잔액이 부족하여 업그레이드할 수 없습니다!");
		printf("[SHOP] 구매 실패: 잔액 부족 (필요: %d원 | 보유: %d원)\n", u->base_cost, g->balance);
		return;
	}

	// 실제 재정 차감 및 스펙 반영
	g->balance -= u->base_cost;
	u->level++;

	if (upg_idx == 0) {
		// 제조 슬롯 확장 반영
		g->slot_count = u->level;
		log_push(g, "제조 슬롯이 성공적으로 확장되었습니다!");
	}
	else if (upg_idx == 1) {
		// 머신 속도 향상 반영
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