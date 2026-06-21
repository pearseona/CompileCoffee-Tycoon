#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 
#include "common.h"
#include "game.h"
#include "customer.h"
#include "brew.h"

/* 메뉴 레시피 및 손님 유형 이름 */
MenuInfo g_menu[MAX_MENU];
const char* g_ing_name[MAX_INGREDIENT] = { "원두", "우유", "시럽", "크림", "얼음" };
const char* g_cust_name[3] = { "직장인", "미식가", "학생" };

/* 게임 데이터 초기화 */
void game_init(Game* g) {
	memset(g, 0, sizeof(Game));
	g->state = STATE_MAIN;
	g->day = 1;
	g->balance = 50000; // 초기 자본금
	g->reputation = 50;
	g->slot_count = 1; // 기본 제조 슬롯
	g->shop_page = 0; // 상점 첫 페이지 설정

	// 초기 재료 재고 설정
	g->stock[ING_BEAN] = 15;
	g->stock[ING_MILK] = 10;
	g->stock[ING_SYRUP] = 5;
	g->stock[ING_CREAM] = 5;
	g->stock[ING_ICE] = 10;

	// 메뉴 레시피 정의: {이름, 판매가, 원가, 제조시간(ms), {재료배합}, 해금여부}
	g_menu[MENU_AMERICANO] = (MenuInfo){ "아메리카노", 4000, 1000, 2000, {2, 0, 0, 0, 1}, 1 };
	g_menu[MENU_LATTE] = (MenuInfo){ "카페라떼", 4500, 1500, 3000, {1, 2, 0, 0, 0}, 1 };
	g_menu[MENU_VANILLA_LATTE] = (MenuInfo){ "바닐라라떼", 5000, 1800, 3500, {1, 1, 2, 0, 0}, 0 };
	g_menu[MENU_COLD_BREW] = (MenuInfo){ "콜드브루", 4800, 1200, 1500, {3, 0, 0, 0, 2}, 0 };
	g_menu[MENU_CARAMEL_MAC] = (MenuInfo){ "카라멜마키아또", 5500, 2000, 4000, {1, 1, 1, 1, 0}, 0 };
	g_menu[MENU_ESPRESSO] = (MenuInfo){ "에스프레소", 3500, 800, 1000, {2, 0, 0, 0, 0}, 1 };

	upg_init(g); // 업그레이드 항목 초기화
	srand((unsigned int)time(NULL)); // 난수 생성 초기화
	log_push(g, "컴파일 커피에 오신 것을 환영합니다!");
}

/* 하루 영업 시작 */
void game_start_day(Game* g) {
	
	g->state = STATE_PLAYING;
	g->day_ms = DAY_SEC * 1000; // 45초 영업 시간

	g->day_revenue = 0;
	g->day_expenditure = 0;
	g->spawn_timer_ms = 5000; // 첫 손님 스폰 딜레이

	g->q_head = 0; g->q_tail = 0; g->q_size = 0;

	// 제조 슬롯 초기화
	for (int i = 0; i < MAX_BREW_SLOTS; i++) {
		g->slots[i].state = SLOT_EMPTY;
	}

	g->combo = 0; g->combo_timer = 0;

	// 재고 자동 배달 쿨다운 초기화
	for (int i = 0; i < MAX_INGREDIENT; i++) {
		g->stock_refill_ms[i] = 0;
		g->is_refilling[i] = 0;
	}

	memset(g->event_msg, 0, sizeof(g->event_msg));
	g->event_ms = 0;

	log_push(g, "새로운 하루 영업을 개시합니다!");

	// 직장인 / 학생 러시 랜덤 결정
	int day_roll = rand() % 100;
	if (day_roll < 25) {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "RUSH_WORKER");
		g->event_ms = 25000;
		log_push(g, "🔥 [피크타임] 점심 직장인 러시 발동! 25초간 손님이 쏟아집니다!");
	}
	else if (day_roll >= 25 && day_roll < 45) {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "RUSH_STUDENT");
		g->event_ms = 20000;
		log_push(g, "⚡ [피크타임] 학생 손님들이 물밀기듯 찾아옵니다!");
	}
	else {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "NORMAL");
		log_push(g, "☕ 매장이 비교적 한산합니다. 여유롭게 장사를 준비하세요.");
	}

	// 이벤트 NPC 등장: 위생검사관 / 인플루언서 등장 확률 처리
	int npc_roll = rand() % 100;
	if (npc_roll < 20) {
		log_push(g, "[이벤트] 위생검사관이 매장을 불시 방문했습니다!");
		if (g->stock[ING_BEAN] < 3 || g->stock[ING_ICE] < 2) {
			g->reputation = clamp_i(g->reputation - 10, 0, 100);
			log_push(g, "위생 상태 및 재고 부족으로 평판이 10 차감되었습니다!");
		}
	}
	else if (npc_roll >= 80) {
		log_push(g, "[이벤트] 유명 인플루언서가 손님 무리에 합류했습니다!");
	}
}

/* 하루 영업 마감 및 최종 결과 판정 */
void game_close_day(Game* g) {
	int profit = g->day_revenue - g->day_expenditure;
	g->balance += profit;
	g->total_profit += profit;

	if (g->day <= MAX_DAYS) {
		g->records[g->day - 1] = (DayRecord){ g->day, g->day_revenue, g->day_expenditure, profit, g->combo };
	}

	// 엔딩 판정
	if (g->day >= MAX_DAYS) {

		// 120만워 달성
		if (g->balance >= GOAL_PROFIT) {
			g->state = STATE_HIGHSCORE; 
			log_push(g, "🎉 축하합니다! 15일간 목표 자산 120만원 달성 대승리!");
		}
		// 실패
		else {
			g->state = STATE_GAMEOVER; 
			log_push(g, "😢 아쉽습니다. 목표 자산 120만원 달성에 실패했습니다.");
		}
		return;
	}

	g->state = STATE_UPGRADE; // 영업 중단 시 정비 화면 이동
}

/* 게임 시스템 실시간 업데이트 */
void game_update(Game* g, Uint32 dt) {

	if (g->state != STATE_PLAYING)
		return;

	// 영업 시간 감소 및 타임아웃 정산
	g->day_ms -= dt;
	if (g->day_ms <= 0) {
		g->day_ms = 0;
		game_close_day(g);
		return;
	}

	// 피크타임 타이머 차감
	if (g->event_ms > 0) {
		if (g->event_ms > (int)dt) g->event_ms -= dt;
		else {
			g->event_ms = 0;
			log_push(g, "✨ 피크타임 러시가 무사히 종료되어 매장이 안정화되었습니다.");
			strcpy_s(g->event_msg, sizeof(g->event_msg), "NORMAL");
		}
	}

	// 콤보 시스템 제어
	if (g->combo > 0 && g->combo < 3) {
		if (g->combo_timer > dt) g->combo_timer -= dt;
		else { g->combo = 0; g->combo_timer = 0; }
	}

	if (g->combo >= 3) {
		if (g->combo_timer > dt) g->combo_timer -= dt;
		else { g->combo = 0; g->combo_timer = 0; }
	}

	cust_update(g, dt);
	brew_update(g, dt);

	// 재고 고갈 시 긴급 배달
	for (int i = 0; i < MAX_INGREDIENT; i++) {
		if (i == ING_BEAN || i == ING_MILK) {
			if (g->stock[i] <= 0) {
				if (!g->is_refilling[i]) {
					g->is_refilling[i] = 1;
					g->stock_refill_ms[i] = 3000;
				}
				g->stock_refill_ms[i] -= dt;
				if (g->stock_refill_ms[i] <= 0) {
					int penalty_cost = (i == ING_BEAN) ? 400 : 250;
					if (g->balance >= penalty_cost) {
						g->balance -= penalty_cost;
						g->stock[i] += 1;
						g->is_refilling[i] = 0;
					}
					else { g->stock_refill_ms[i] = 1000; }
				}
			}
			else g->is_refilling[i] = 0;
		}
	}
}

/* 시스템 로그 메시지 */
void log_push(Game* g, const char* msg) {
	if (!msg) return;
	int idx = g->log_count % MAX_LOG_LINES;
	strcpy_s(g->log_lines[idx], sizeof(g->log_lines[idx]), msg);
	g->log_ttl[idx] = SDL_GetTicks() + 4000;
	g->log_count++;
}

/* 점수 값의 범위를 특정 구간으로 제한 */
int clamp_i(int v, int lo, int hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}