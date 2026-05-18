// 게임의 메인 시작점
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

MenuInfo g_menu[MAX_MENU];
const char* g_ing_name[MAX_INGREDIENT] = { "원두", "우유", "시럽", "크림", "얼음" }; // 제료
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
	strcpy(g->upg[0].name, "제조 슬롯 확장");
	strcpy(g->upg[0].desc, "동시에 음료를 제조할 수 있는 슬롯을 추가합니다.");
	g->upg[0].base_cost = 3000;
	g->upg[0].level = 1;
	g->upg[0].max_level = MAX_BREW_SLOTS;

	strcpy(g->upg[1].name, "머신 속도 향상");
	strcpy(g->upg[1].desc, "음료 제조 속도가 15% 빨라집니다.");
	g->upg[1].base_cost = 25000; 
	g->upg[1].level = 0; 
	g->upg[1].max_level = 3;

	log_push(g, "컴파일 커피에 오신 것을 환영합니다!");
}

/* 새로운 날 영업 시작 */
void game_start_day(Game* g) {
	g->state = STATE_PLAYING;
	g->day_ms = DAY_SEC * 1000; // 90초를 ms 단위로 환산 (90000ms)
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

	log_push(g, "새로운 하루 영업을 개시합니다!");
}

/* 하루 영업 마감 및 일별 레코드 파일 백업 준비 */
void game_close_day(Game* g) {
	g->state = STATE_CLOSING;

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
	log_push(g, "영업 마감! 정산 화면으로 이동합니다.");
}

/* 실시간 시간 경과 처리 */
void game_update(Game* g, Uint32 dt) {
	if (g->state != STATE_PLAYING)
		return;

	// 영업 시간 차감 (90초 제한 시간)
	g->day_ms -= dt;
	if (g->day_ms <= 0) {
		g->day_ms = 0;
		game_close_day(g);
		return;
	}

	// 콤보 제한시간 타이머 갱신
	if (g->combo > 0) {
		if (SDL_GetTicks() - g->combo_timer > COMBO_TIMEOUT_MS) {
			g->combo = 0;
			log_push(g, "콤보 타이아웃! 콤보가 초기화되었습니다.");
		}
	}

	// 손님 스폰 및 인내심 틱 업데이트 (추후 예정)
	cust_update(g, dt);

	// 음료 제조 진행 상태 업데이트 (추후 예정)
	brew_update(g, dt);
}

/* 실시간 알림 로그 푸시 */
void log_push(Game* g, const char* msg) {
	// 알림창에 빈 자리가 있는 경우 (6줄 미만)
	if (g->log_count < LOG_MAX) {
		strcpy(g->log_lines[g->log_count], msg);
		g->log_ttl[g->log_count] = SDL_GetTicks() + 4000; // 4초 동안 보여주기
		g->log_count++;
	}
	// 이미 알림창이 6줄로 꽉 찬 경우
	else {
		// 한 줄씩 위로 당기기
		for (int i = 1; i < LOG_MAX; i++) {
			strcpy(g->log_lines[i - 1], g->log_lines[i]);
			g->log_ttl[i - 1] = g->log_ttl[i];
		}
		strcpy(g->log_lines[LOG_MAX - 1], msg);
		g->log_ttl[LOG_MAX - 1] = SDL_GetTicks() + 4000;
	}
}

// 정수 값 범위 제한
int clamp_i(int v, int lo, int hi) {
	if (v < lo) return lo; // 최소치로 고정
	if (v > hi) return hi; // 최대치로 고정
	return v;
}