#define _CRT_SECURE_NO_WARNINGS
#include "common.h"
#include "brew.h"
#include <stdio.h>

/* 특정 제조 슬롯에서 음료 제조를 개시 */
void brew_start(Game* g, int slot_idx, int qi, MenuID menu) {

	// 슬롯 유효성 검사
	if (slot_idx < 0 || slot_idx >= g->slot_count) {
		log_push(g, "상점에서 슬롯을 먼저 확장해 주세요!");
		printf("[WARN] 제조 실패: 보유 슬롯 초과 접근 (요구: %d / 보유: %d)\n", slot_idx + 1, g->slot_count);
		return;
	}

	// 슬롯 상태 확인
	if (g->slots[slot_idx].state != SLOT_EMPTY) {
		log_push(g, "해당 슬롯은 이미 음료를 제조 중입니다!");
		return;
	}

	// 재고 검증
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

	// 재고 자동 소모
	for (int i = 0; i < MAX_INGREDIENT; i++) {
		g->stock[i] -= g_menu[menu].ing[i];
	}

	// 업그레이드 보너스 계산: 머신 속도 강화 레벨당 15% 가속 적용
	// 기본 제조 시간에서 레벨당 15% 단축, 최소 속도 마지노선 40% 방어
	int final_brew_ms = g_menu[menu].brew_ms;
	if (g->upg[1].level > 0) {
		double speed_bonus = 1.0 - (g->upg[1].level * 0.15);
		if (speed_bonus < 0.4) speed_bonus = 0.4; 
		final_brew_ms = (int)(final_brew_ms * speed_bonus);
	}

	// 제조 슬롯 데이터 세팅
	g->slots[slot_idx].state = SLOT_BREWING;
	g->slots[slot_idx].menu = menu;
	g->slots[slot_idx].elapsed_ms = 0; // 제조 경과 시간 초기화
	g->slots[slot_idx].required_ms = final_brew_ms; // 최종 보정된 제조 목표 시간 설정
	g->slots[slot_idx].cust_id = g->queue[qi].id; // 손님 고유 ID 식별자 매핑

	// 제조 시작 로그 및 콘솔 디버깅
	char msg[80];
	sprintf_s(msg, sizeof(msg), "[%s] 제조를 시작합니다...", g_menu[menu].name);
	log_push(g, msg);

	printf("[BREW] %d번 슬롯: [%s] 가동 개시 (필요시간: %dms, 손님ID: %d)\n",
		slot_idx + 1, g_menu[menu].name, g->slots[slot_idx].required_ms, g->slots[slot_idx].cust_id);
}

/* 제조 진행률 업데이트 */
void brew_update(Game* g, Uint32 dt) {

	// 활성화된 제조 슬롯 개수만큼 상태 업데이트
	for (int i = 0; i < g->slot_count; i++) {

		// 현재 음료 제조중
		if (g->slots[i].state == SLOT_BREWING) {
			g->slots[i].elapsed_ms += dt; // 경과 시간 누적

			// 메뉴 제조 완료 시간에 도달
			if (g->slots[i].elapsed_ms >= g->slots[i].required_ms) {
				g->slots[i].state = SLOT_DONE; // 제조 완료

				char msg[80];
				sprintf_s(msg, sizeof(msg), "[%s] 제조 완료! 서빙 가능합니다.", g_menu[g->slots[i].menu].name);
				log_push(g, msg);

				printf("[DONE] %d번 슬롯: %s 완성! (소요: %dms)\n",
					i + 1, g_menu[g->slots[i].menu].name, g->slots[i].elapsed_ms);
			}
		}
	}
}

/* 제조 중인 슬롯 강제 취소(원자재는 폐기) */
void brew_cancel(Game* g, int slot_idx) {

	if (slot_idx < 0 || slot_idx >= MAX_BREW_SLOTS) return;

	// 상태 리셋
	if (g->slots[slot_idx].state != SLOT_EMPTY) {
		g->slots[slot_idx].state = SLOT_EMPTY; // 슬롯 비우기
		log_push(g, "음료 제조를 취소했습니다. (재료는 폐기됩니다.)");
		printf("[CANCEL] %d번 슬롯 제조 취소 완료.\n", slot_idx + 1);
	}
}