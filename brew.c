#define _CRT_SECURE_NO_WARNINGS
#include "common.h"
#include "brew.h"
#include <stdio.h>

/* 특정 제조 슬롯에서 음료 제조를 개시 */
void brew_start(Game* g, int slot_idx, int qi, MenuID menu) {

	// 슬롯 인덱스 범위 예외 처리 및 중복 방지
	if (slot_idx < 0 || slot_idx >= MAX_BREW_SLOTS) return;
	if (g->slots[slot_idx].state != SLOT_EMPTY) {
		log_push(g, "해당 슬롯은 이미 음료를 제조 중입니다!");
		return;
	}

	// 원자재 재고가 충분한지 검사
	for (int i = 0; i < MAX_INGREDIENT; i++) {
		int required = g_menu[menu].ing[i];
		if (g->stock[i] < required) {
			// 재고 부족 시 경고 알림창
			log_push(g, "재고가 부족하여 음료를 제조할 수 없습니다!");
			printf("[WARN] 제조 실패: %s 재고 부족 (필요: %d / 보유: %d)\n",
				g_ing_name[i], required, g->stock[i]);
			return;
		}
	}

	// 재고가 충분하면 자동 소모
	for (int i = 0; i < MAX_INGREDIENT; i++) {
		g->stock[i] -= g_menu[menu].ing[i];
	}

	// 제조 슬롯 데이터 세팅
	g->slots[slot_idx].state = SLOT_BREWING;
	g->slots[slot_idx].menu = menu;
	g->slots[slot_idx].elapsed_ms = 0; // 진행 시간 초기화
	g->slots[slot_idx].required_ms = g_menu[menu].brew_ms; // 메뉴 제조시간
	g->slots[slot_idx].cust_id = qi; // 주문한 손님 ID 

	// 알림 로그
	char msg[80];
	sprintf_s(msg, "[%s] 제조를 시작합니다...", g_menu[menu].name);
	log_push(g, msg);

	printf("[BREW] %d번 슬롯: [%s] 가동 개시 (필요시간: %dms)\n",
		slot_idx + 1, g_menu[menu].name, g->slots[slot_idx].required_ms);
}

// 제조 진행 시간을 실시간 누적
void brew_update(Game* g, Uint32 dt) {

	// 현재 플레이어가 해금해서 쓰고 있는 슬롯 개수만큼 순회
	for (int i = 0; i < g->slot_count; i++) {

		// 현재 음료 제조중
		if (g->slots[i].state == SLOT_BREWING) {
			g->slots[i].elapsed_ms += dt; // 델타 타임 실시간 누적

			// 메뉴 제조 완료 시간에 도달
			if (g->slots[i].elapsed_ms >= g->slots[i].required_ms) {
				g->slots[i].state = SLOT_DONE; // 제조 완료

				char msg[80];

				sprintf_s(msg, "[%s] 제조 완료! 서빙 가능합니다.", g_menu[g->slots[i].menu].name);
				log_push(g, msg);

				printf("[DONE] %d번 슬롯: %s 완성! (소요: %dms)\n",
					i + 1, g_menu[g->slots[i].menu].name, g->slots[i].elapsed_ms);
			}
		}
	}
}

// 제조 중인 슬롯 강제 취소 (원자재는 버려짐)
void brew_cancel(Game* g, int slot_idx) {

	if (slot_idx < 0 || slot_idx >= MAX_BREW_SLOTS) return;

	if (g->slots[slot_idx].state != SLOT_EMPTY) {
		g->slots[slot_idx].state = SLOT_EMPTY; // 슬롯 비우기
		log_push(g, "음료 제조를 취소했습니다. (재료는 폐기됩니다.)");
		printf("[CANCEL] %d번 슬롯 제조 취소 완료.\n", slot_idx + 1);
	}
}