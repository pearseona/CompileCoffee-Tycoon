#define _CRT_SECURE_NO_WARNINGS
#include "common.h"
#include "customer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 대기열 및 타이머 초기화 */
void cust_init(Game* g) {
	for (int i = 0; i < MAX_QUEUE; i++) {
		g->queue[i].active = 0;
	}
	g->spawn_timer_ms = 0;
}

/* 델타 타임을 누적 및 조건별 손님 생성 */
void cust_spawn(Game* g, Uint32 dt) {
	g->spawn_timer_ms += dt;

	// 3일씩 지날 때마다 스폰 속도 1초씩 단축 가속 알고리즘
	int speed_bonus_level = (g->day - 1) / 3;
	int dynamic_base_interval = SPAWN_INTERVAL_MS - (speed_bonus_level * 1000);

	if (dynamic_base_interval < 3000) {
		dynamic_base_interval = 3000;
	}

	int current_spawn_interval = dynamic_base_interval;

	if (g->event_ms > 0 && g->event_msg[0] != '\0') {
		if (strcmp(g->event_msg, "RUSH_WORKER") == 0 || strcmp(g->event_msg, "RUSH_STUDENT") == 0) {
			current_spawn_interval = 3200;
		}
	}

	if (g->spawn_timer_ms >= current_spawn_interval) {
		g->spawn_timer_ms = 0;

		for (int i = 0; i < 3; i++) {
			if (g->queue[i].active == 0) {
				g->queue[i].active = 1;

				CustType new_type;
				if (g->event_ms > 0 && g->event_msg[0] != '\0' && strcmp(g->event_msg, "RUSH_WORKER") == 0) {
					new_type = (rand() % 100 < 80) ? CUST_WORKER : (CustType)(rand() % CUST_TYPE_COUNT);
				}
				else if (g->event_ms > 0 && g->event_msg[0] != '\0' && strcmp(g->event_msg, "RUSH_STUDENT") == 0) {
					new_type = (rand() % 100 < 80) ? CUST_STUDENT : (CustType)(rand() % CUST_TYPE_COUNT);
				}
				else {
					new_type = (CustType)(rand() % 3);
				}

				g->queue[i].type = new_type;

				// 🎯 [오타 교정 완료]: payout_max 접근 구문을 완전히 삭제 및 구조체 규격에 맞춤
				if (new_type == CUST_WORKER) {
					g->queue[i].patience_max = 8000;
					g->queue[i].order = (rand() % 2); // MENU_AMERICANO 또는 MENU_LATTE
					log_push(g, "💼 [🚨] 직장인 \"아점 커피 급해요! 2배 빨리 만들어라냥!\"");
				}
				else if (new_type == CUST_FOODIE) {
					g->queue[i].patience_max = 20000;

					MenuID premium_pool[6];
					int p_count = 0;
					if (g_menu[MENU_VANILLA_LATTE].unlocked) premium_pool[p_count++] = MENU_VANILLA_LATTE;
					if (g_menu[MENU_COLD_BREW].unlocked) premium_pool[p_count++] = MENU_COLD_BREW;
					if (g_menu[MENU_CARAMEL_MAC].unlocked) premium_pool[p_count++] = MENU_CARAMEL_MAC;

					if (p_count > 0) {
						g->queue[i].order = premium_pool[rand() % p_count];
					}
					else {
						g->queue[i].order = MENU_LATTE;
					}
					log_push(g, "👑 [👑] 미식가 \"음료 퀄리티를 보러 왔다냥.\"");
				}
				else if (new_type == CUST_STUDENT) {
					g->queue[i].patience_max = 15000;
					MenuID stud_options[] = { MENU_AMERICANO, MENU_LATTE, MENU_ESPRESSO };
					g->queue[i].order = stud_options[rand() % 3];
					log_push(g, "🎓 [🎓] 학 생 \"주머니 사정이 가볍다냥. 열공 모드 충전!\"");
				}

				g->queue[i].patience_ms = g->queue[i].patience_max;
				g->queue[i].id = g->next_id++;
				g->queue[i].served = 0;
				break;
			}
		}
	}
}

/* 저장된 큐 인덱스의 손님 정산 및 퇴장 처리 */
void game_serve_drink(Game* g, int customer_idx) {
	if (customer_idx < 0 || customer_idx >= 3) return;

	Customer* c = &g->queue[customer_idx];
	int menu_price = g_menu[c->order].price;
	int final_payout = menu_price;

	if (c->type == CUST_STUDENT) {
		final_payout = (int)(menu_price * 0.8);
		g->reputation = clamp_i(g->reputation + 2, 0, 100);
		log_push(g, "🎓 학생 응원 10% 할인이 자동 정산되었습니다! (평판 +2)");
	}
	else if (c->type == CUST_FOODIE) {
		final_payout = (int)(menu_price * 1.5);
		g->reputation = clamp_i(g->reputation + 5, 0, 100);
		log_push(g, "👑 미식가가 특급 퀄리티에 대만족하여 50% 팁을 투척했습니다! (평판 +5)");
	}
	else {
		g->reputation = clamp_i(g->reputation + 1, 0, 100);
		log_push(g, "💼 직장인 서빙 성공! 업무에 복귀할 수 있어 안심합니다. (평판 +1)");
	}

	g->balance += final_payout;
	g->day_revenue += final_payout;

	c->active = 0;
	c->served = 1;
}

/* 실시간 인내심 차감 및 타임아웃 예외 처리 */
void cust_update(Game* g, Uint32 dt) {
	cust_spawn(g, dt);

	for (int i = 0; i < 3; i++) {
		if (g->queue[i].active == 1) {
			Customer* c = &g->queue[i];

			if (c->type == CUST_WORKER) {
				c->patience_ms -= (dt * 2);
			}
			else if (c->type == CUST_STUDENT) {
				c->patience_ms -= (int)(dt * 0.7f);
			}
			else {
				c->patience_ms -= dt;
			}

			if (c->type == CUST_FOODIE && c->patience_ms < (c->patience_max / 2)) {
				c->active = 0;
				c->served = -1;
				g->combo = 0;
				g->reputation = clamp_i(g->reputation - 4, 0, 100);

				log_push(g, "❌ 미식가: \"절반이나 기다렸는데 소식이 없군요!\" (중도 탈주, 평판 -4)");
				if (g->sel_cust == i) g->sel_cust = -1;
				continue;
			}

			if (c->patience_ms <= 0) {
				c->active = 0;
				c->served = -1;
				g->combo = 0;

				if (c->type == CUST_WORKER) {
					g->reputation = clamp_i(g->reputation - 6, 0, 100);
					log_push(g, "😡 [타임아웃] 직장인이 평판 테러를 남기고 광속 탈주했습니다! (평판 -6)");
				}
				else {
					g->reputation = clamp_i(g->reputation - 2, 0, 100);
					log_push(g, "😢 [타임아웃] 손님이 기다리다 지쳐서 가버렸습니다... (평판 -2)");
				}

				printf("[WARN] %d번 자리 손님 타임아웃 퇴장. 콤보가 초기화됩니다.\n", i + 1);
				if (g->sel_cust == i) {
					g->sel_cust = -1;
				}
			}
		}
	}
}

/* 큐 인덱스로 손님 데이터 주소 반환 */
Customer* cust_at(Game* g, int qi) {
	if (qi >= 0 && qi < 3 && g->queue[qi].active) {
		return &g->queue[qi];
	}
	return NULL;
}

/* 제조 슬롯 완료 시 호출될 서빙 */
void cust_serve(Game* g, int slot_idx) {
	if (slot_idx < 0 || slot_idx >= g->slot_count) return;
	if (g->slots[slot_idx].state != SLOT_DONE) {
		log_push(g, "아직 완료되지 않은 슬롯입니다!");
		return;
	}

	int target_cust = g->sel_cust;
	if (target_cust < 0 || target_cust >= 3 || g->queue[target_cust].active == 0) {
		log_push(g, "서빙할 손님을 먼저 마우스로 클릭하여 선택해 주세요!");
		return;
	}

	Customer* c = &g->queue[target_cust];
	MenuID cooked_menu = g->slots[slot_idx].menu;
	MenuID ordered_menu = c->order;

	if (cooked_menu == ordered_menu) {
		if (g->combo >= 3) {
			log_push(g, "판매 성공! (콤보 쿨타임 제한 중...)");
		}
		else {
			g->combo++;
			g->combo_timer = COMBO_TIMEOUT_MS;

			if (g->combo == 3) {
				g->combo_timer = 5000;
				log_push(g, "3콤보 달성! 5초간 콤보 시스템이 잠깁니다.");
				printf("[COMBO] ★★★ 3콤보 달성! 5초간 쿨다운 잠금 ★★★\n");
			}
		}

		game_serve_drink(g, target_cust);
		printf("[SUCCESS] 서빙 성공! 손님 주문: %s | 현재 콤보: %d\n", g_menu[ordered_menu].name, g->combo);
	}
	else {
		log_push(g, "주문과 다른 음료입니다! 재료가 낭비되었습니다.");
		g->combo = 0;

		printf("[FAIL] 서빙 실패: 손님 요구[%s] != 바리스타 제조[%s] | 콤보 리셋\n",
			g_menu[ordered_menu].name, g_menu[cooked_menu].name);
	}

	g->slots[slot_idx].state = SLOT_EMPTY;
	g->sel_cust = -1;
}