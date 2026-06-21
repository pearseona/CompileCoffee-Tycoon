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

MenuInfo g_menu[MAX_MENU];
const char* g_ing_name[MAX_INGREDIENT] = { "원두", "우유", "시럽", "크림", "얼음" };
const char* g_cust_name[3] = { "직장인", "미식가", "학생" };

void game_init(Game* g) {
	memset(g, 0, sizeof(Game));
	g->state = STATE_MAIN;
	g->day = 1;
	g->balance = 50000;
	g->reputation = 50;
	g->slot_count = 1;
	g->shop_page = 0;

	g->stock[ING_BEAN] = 15;
	g->stock[ING_MILK] = 10;
	g->stock[ING_SYRUP] = 5;
	g->stock[ING_CREAM] = 5;
	g->stock[ING_ICE] = 10;

	g_menu[MENU_AMERICANO] = (MenuInfo){ "아메리카노", 4000, 1000, 2000, {2, 0, 0, 0, 1}, 1 };
	g_menu[MENU_LATTE] = (MenuInfo){ "카페라떼", 4500, 1500, 3000, {1, 2, 0, 0, 0}, 1 };
	g_menu[MENU_VANILLA_LATTE] = (MenuInfo){ "바닐라라떼", 5000, 1800, 3500, {1, 1, 2, 0, 0}, 0 };
	g_menu[MENU_COLD_BREW] = (MenuInfo){ "콜드브루", 4800, 1200, 1500, {3, 0, 0, 0, 2}, 0 };
	g_menu[MENU_CARAMEL_MAC] = (MenuInfo){ "카라멜마키아또", 5500, 2000, 4000, {1, 1, 1, 1, 0}, 0 };
	g_menu[MENU_ESPRESSO] = (MenuInfo){ "에스프레소", 3500, 800, 1000, {2, 0, 0, 0, 0}, 1 };

	upg_init(g);
	srand((unsigned int)time(NULL));
	log_push(g, "컴파일 커피에 오신 것을 환영합니다!");
}

void game_start_day(Game* g) {
	g->state = STATE_PLAYING;

	// ⏱️ 기획서 동기화: 하루 45초 영업 사양 충전 연동
	g->day_ms = DAY_SEC * 1000;

	g->day_revenue = 0;
	g->day_expenditure = 0;
	g->spawn_timer_ms = 5000;

	g->q_head = 0; g->q_tail = 0; g->q_size = 0;

	for (int i = 0; i < MAX_BREW_SLOTS; i++) {
		g->slots[i].state = SLOT_EMPTY;
	}

	g->combo = 0; g->combo_timer = 0;

	for (int i = 0; i < MAX_INGREDIENT; i++) {
		g->stock_refill_ms[i] = 0;
		g->is_refilling[i] = 0;
	}

	memset(g->event_msg, 0, sizeof(g->event_msg));
	g->event_ms = 0;

	log_push(g, "새로운 하루 영업을 개시합니다!");

	int day_roll = rand() % 100;
	if (day_roll < 25) {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "RUSH_WORKER");
		g->event_ms = 25000;
		log_push(g, "🔥 [피크타임] 점심 직장인 러시 발동! 25초간 손님이 쏟아집니다!");
	}
	else if (day_roll >= 25 && day_roll < 45) {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "RUSH_STUDENT");
		g->event_ms = 20000;
		log_push(g, "⚡ [피크타임] 대학 종강일! 학생 손님들이 물밀기듯 찾아옵니다!");
	}
	else {
		strcpy_s(g->event_msg, sizeof(g->event_msg), "NORMAL");
		log_push(g, "☕ 매장이 비교적 한산합니다. 여유롭게 장사를 준비하세요.");
	}

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

/* 하루 마감 정산 처리 */
void game_close_day(Game* g) {
	int profit = g->day_revenue - g->day_expenditure;
	g->balance += profit;
	g->total_profit += profit;

	if (g->day <= MAX_DAYS) {
		g->records[g->day - 1] = (DayRecord){ g->day, g->day_revenue, g->day_expenditure, profit, g->combo };
	}

	// 🎯 [기획 개편 핵심]: 오늘이 최종 15일 차였다면 즉시 엔딩 정산 판정 분기로 우회!
	if (g->day >= MAX_DAYS) {
		if (g->balance >= GOAL_PROFIT) {
			g->state = STATE_HIGHSCORE; // 🏆 120만원 이상 달성: 대우승 엔딩 화면으로 지정 워프
			log_push(g, "🎉 축하합니다! 15일간 목표 자산 120만원 달성 대승리!");
		}
		else {
			g->state = STATE_GAMEOVER;  // ❌ 120만원 미만: 목표 달성 실패 화면 워프
			log_push(g, "😢 아쉽습니다. 목표 자산 120만원 달성에 실패했습니다.");
		}
		return;
	}

	// 15일 차 미만일 때는 상점으로 정상 정비 이동
	g->state = STATE_UPGRADE;
}

void game_update(Game* g, Uint32 dt) {
	if (g->state != STATE_PLAYING)
		return;

	g->day_ms -= dt;
	if (g->day_ms <= 0) {
		g->day_ms = 0;
		game_close_day(g);
		return;
	}

	if (g->event_ms > 0) {
		if (g->event_ms > (int)dt) g->event_ms -= dt;
		else {
			g->event_ms = 0;
			log_push(g, "✨ 피크타임 러시가 무사히 종료되어 매장이 안정화되었습니다.");
			strcpy_s(g->event_msg, sizeof(g->event_msg), "NORMAL");
		}
	}

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

void log_push(Game* g, const char* msg) {
	if (!msg) return;
	int idx = g->log_count % MAX_LOG_LINES;
	strcpy_s(g->log_lines[idx], sizeof(g->log_lines[idx]), msg);
	g->log_ttl[idx] = SDL_GetTicks() + 4000;
	g->log_count++;
}

int clamp_i(int v, int lo, int hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}