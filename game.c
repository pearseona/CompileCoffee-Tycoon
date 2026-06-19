// 게임의 메인 시작점
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> // 🛠️ 랜덤 시간(time)을 쓰기 위해 표준 헤더 추가 완료!
#include "common.h"
#include "game.h"
#include "customer.h"
#include "brew.h"

MenuInfo g_menu[MAX_MENU];
const char* g_ing_name[MAX_INGREDIENT] = { "원두", "우유", "시럽", "크림", "얼음" }; // 재료
const char* g_cust_name[3] = { "직장인", "미식가", "학생" }; // 손님 유형

/* 게임 최초 실행 시 데이터 초기화 (Menu 레시피 & 초기 자본 셋업) */
void game_init(Game* g) {
	memset(g, 0, sizeof(Game));
	g->state = STATE_MAIN;
	g->day = 1;
	g->balance = 50000; // 초기 자본금 5만원
	g->reputation = 50; // 초기 평판 
	g->slot_count = 1; // 기본 제조 슬롯은 1개부터 시작

	// 초기 재료 재고 
	g->stock[ING_BEAN] = 15;
	g->stock[ING_MILK] = 10;
	g->stock[ING_SYRUP] = 5;
	g->stock[ING_CREAM] = 5;
	g->stock[ING_ICE] = 10;

	// 메뉴 레시피 
	// 판매가, 원가, 제조시간(ms), {원두, 우유, 시럽, 크림, 얼음}, 해금여부
	// 아메리카노 (원두 2, 얼음 1)
	g_menu[MENU_AMERICANO] = (MenuInfo){ "아메리카노", 4000, 1000, 2000, {2, 0, 0, 0, 1}, 1 };
	// 카페라떼 (원두 1, 우유 2)
	g_menu[MENU_LATTE] = (MenuInfo){ "카페라떼", 4500, 1500, 3000, {1, 2, 0, 0, 0}, 1 };
	// 바닐라라떼 (원두 1, 우유 1, 시럽 2)
	g_menu[MENU_VANILLA_LATTE] = (MenuInfo){ "바닐라라떼", 5000, 1800, 3500, {1, 1, 2, 0, 0}, 0 }; // 초기 잠금
	// 콜드브루 (원두 3, 얼음 2)
	g_menu[MENU_COLD_BREW] = (MenuInfo){ "콜드브루", 4800, 1200, 1500, {3, 0, 0, 0, 2}, 0 };   // 초기 잠금 
	// 카라멜 마키아토 (원두 1, 우유 1, 시럽 1, 크림 1)
	g_menu[MENU_CARAMEL_MAC] = (MenuInfo){ "카라멜마키아토", 5500, 2000, 4000, {1, 1, 1, 1, 0}, 0 }; // 초기 잠금
	// 에스프레소 (원두 2)
	g_menu[MENU_ESPRESSO] = (MenuInfo){ "에스프레소", 3500, 800, 1000, {2, 0, 0, 0, 0}, 1 };

	// 상점 업그레이드 품목 초기화 
	strcpy_s(g->upg[0].name, sizeof(g->upg[0].name), "제조 슬롯 확장");
	strcpy_s(g->upg[0].desc, sizeof(g->upg[0].desc), "동시에 음료를 제조할 수 있는 슬롯을 추가합니다.");
	g->upg[0].base_cost = 3000;
	g->upg[0].level = 1;
	g->upg[0].max_level = MAX_BREW_SLOTS;

	strcpy_s(g->upg[1].name, sizeof(g->upg[1].name), "머신 속도 향상");
	strcpy_s(g->upg[1].desc, sizeof(g->upg[1].desc), "음료 제조 속도가 15% 빨라집니다.");
	g->upg[1].base_cost = 25000;
	g->upg[1].level = 0;
	g->upg[1].max_level = 3;

	// 손님, 이벤트 랜덤
	srand((unsigned int)time(NULL));

	log_push(g, "컴파일 커피에 오신 것을 환영합니다!");
}

/* 새로운 날 영업 시작 */
void game_start_day(Game* g) {
	g->state = STATE_PLAYING;

	// ⏱️ 기획서 동기화: 정확히 90초(90000ms) 영업 시간 풀 충전 세팅!
	g->day_ms = DAY_SEC * 1000;

	g->day_revenue = 0;
	g->day_expenditure = 0;
	g->spawn_timer_ms = 2000; // 영업 시작 2초 뒤 첫 손님 리액션

	// 원형 큐 대기열 초기화
	g->q_head = 0;
	g->q_tail = 0;
	g->q_size = 0;

	// 제조 슬롯 비우기
	for (int i = 0; i < MAX_BREW_SLOTS; i++) {
		g->slots[i].state = SLOT_EMPTY;
	}

	// 쿨다운 타이머 초기화
	g->combo = 0;
	g->combo_timer = 0;

	log_push(g, "새로운 하루 영업을 개시합니다!");

	/* 이벤트 NPC 확률 시스템 */
	int npc_roll = rand() % 100;

	if (npc_roll < 20) {
		// 20% 확률로 위생검사관 등장 (재고 불시 검문)
		log_push(g, "[이벤트] 위생검사관이 매장을 불시 방문했습니다!");
		log_push(g, "재고 상태가 불량할 경우 평판 페널티가 부여됩니다.");
		printf("\n[EVENT] 위생검사관 습격. 재고 관리 경보 발령!\n\n");

		// 위생검사관 페널티: 특정 재고가 바닥나 있으면 평판 차감
		if (g->stock[ING_BEAN] < 3 || g->stock[ING_ICE] < 2) {
			g->reputation = clamp_i(g->reputation - 10, 0, 100);
			log_push(g, "위생 상태 및 재고 부족으로 평판이 10 차감되었습니다!");
		}
	}
	else if (npc_roll >= 80) {
		// 20% 확률로 인플루언서 입장 
		log_push(g, "[이벤트] 유명 인플루언서가 손님 무리에 합류했습니다!");
		log_push(g, "주문 성공 시 평판이 폭등하지만, 실패 시 폭락합니다!");
		printf("\n[EVENT] 인플루언서 방문. 대박 혹은 쪽박 기회!\n\n");
	}
}

/* 하루 영업 마감 및 일별 레코드 파일 백업 준비 */
void game_close_day(Game* g) {
	g->state = STATE_UPGRADE; // 영업 종료 후 자동으로 상점/정비 화면으로 원활하게 자동 워프!

	int profit = g->day_revenue - g->day_expenditure;
	g->balance += profit;
	g->total_profit += profit;

	// 레코드 배열에 저장
	if (g->day <= MAX_DAYS) {
		g->records[g->day - 1] = (DayRecord){
			g->day,
			g->day_revenue,
			g->day_expenditure,
			profit,
			g->combo
		};
	}
	log_push(g, "90초 영업 마감! 상점 및 정산 화면으로 자동 이동했습니다.");
}

/* 실시간 시간 경과 처리 */
void game_update(Game* g, Uint32 dt) {
	if (g->state != STATE_PLAYING)
		return;

	// 영업 시간 차감 (90초 제한 시간 실시간 연동)
	g->day_ms -= dt;
	if (g->day_ms <= 0) {
		g->day_ms = 0;
		game_close_day(g); // 90초 타임아웃 시 강제 상점 연계 함수 가동
		return;
	}

	/* 콤보 유지 및 락다운 타이머 */
	if (g->combo > 0 && g->combo < 3) {
		if (g->combo_timer > dt) {
			g->combo_timer -= dt;
		}
		else {
			g->combo = 0;
			g->combo_timer = 0;
			log_push(g, "콤보 타임아웃! 연속 보너스가 초기화되었습니다.");
			printf("[COMBO] 제한 시간 만료로 인해 콤보 리셋.\n");
		}
	}

	// 3콤보 달성하면 5초동안 락 상태
	if (g->combo >= 3) {
		if (g->combo_timer > dt) {
			g->combo_timer -= dt; // 5초 쿨다운 실시간 차감
		}
		else {
			g->combo = 0;
			g->combo_timer = 0;
			log_push(g, "콤보 락다운이 해제되었습니다! 다시 콤보를 노리세요.");
			printf("[COMBO] 5초 쿨다운 만료! 잠금 해제 완료.\n");
		}
	}

	// 손님 스폰 및 인내심 틱 업데이트
	cust_update(g, dt);

	// 음료 제조 진행 상태 업데이트 
	brew_update(g, dt);
}

/* 🛠️ 실시간 알림 로그 푸시 (render.c 출력 규격에 100% 연동 동기화 패치) */
void log_push(Game* g, const char* msg) {
	if (!msg) return;

	// 💡 덮어쓰기 오버플로우 방지: 최대 한계선(MAX_LOG_LINES)을 순환형 인덱스로 추적해 안전하게 누적
	int idx = g->log_count % MAX_LOG_LINES;

	strcpy_s(g->log_lines[idx], sizeof(g->log_lines[idx]), msg);
	g->log_ttl[idx] = SDL_GetTicks() + 4000; // 4초 유효시간 기입
	g->log_count++; // 카운트를 정직하게 계속 누적하여 render.c가 최신 글을 정확히 뽑아가도록 유도
}

// 정수 값 범위 제한
int clamp_i(int v, int lo, int hi) {
	if (v < lo) return lo; // 최소치로 고정
	if (v > hi) return hi; // 최대치로 고정
	return v;
}