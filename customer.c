#include "common.h"
#include "customer.h"
#include <stdlib.h>

/* 대기열 및 타이머 초기화 */
void cust_init(Game* g) {
	for (int i = 0; i < MAX_QUEUE; i++) {
		g->queue[i].active = 0; // 모든 대기열에 슬롯 비우기
	}
	g->spawn_timer_ms = 0;
}

/* 델타 타임을 누적 및 조건별 손님 생성 */
void cust_spawn(Game* g, Uint32 dt) {
	g->spawn_timer_ms += dt;

	if (g->spawn_timer_ms >= SPAWN_INTERVAL_MS) {
		g->spawn_timer_ms = 0;

		for (int i = 0; i < MAX_QUEUE; i++) {
			if (g->queue[i].active == 0) {

				g->queue[i].active = 1;

				// 3가지 단골 손님 유형 중 랜덤 결정
				// 0: 직장인, 1: 미식가, 2: 학생
				CustType new_type = (CustType)(rand() % CUST_TYPE_COUNT);
				g->queue[i].type = new_type;

				// 유형별 특징
				if (new_type == CUST_WORKER) {
					// 직장인: 인내심 8초, 대신 아무거나 잘 마심
					g->queue[i].patience_max = 8000;
					g->queue[i].order = (rand() % 2); // 기본 음료 위주
					printf("[입장] %d번 자리에 [직장인] 입장! (인내심 매우 짧음!)\n", i+1);
					log_push(g, "바쁜 직장인이 입장했습니다! 서두르세요!");
				
				}
				else if (new_type == CUST_FOODIE) {
					// 미식가: 인내심 20초, 대신 무조건 비싼 메뉴만 주문
					g->queue[i].patience_max = 20000;
					g->queue[i].order = MENU_ESPRESSO;
					printf("[입장] %d번 자리에 [미식가] 입장! (고급 메뉴 요구)\n", i+1);
					log_push(g, "까다로운 미식가가 입장했습니다. 고급 음료를 주문합니다.");
				
				}
				else if (new_type == CUST_STUDENT) {
					// 학생: 인내심 14초, 아무거나 시킴
					g->queue[i].patience_max = 15000;
					g->queue[i].order = (rand() % 3) + 1;
					printf("[입장] %d번 자리에 [학생] 입장! (정산 시 할인 혜택 필요)\n", i+1);
					log_push(g, "할인 혜택이 필요한 학생이 입장했습니다.");
				}

				// 인내심 초기화
				g->queue[i].patience_ms = g->queue[i].patience_max;
				break;
			}
		}
	}
}

/* 저장된 큐 인덱스의 손님 정산 및 퇴장 처리 */
void game_serve_drink(Game* g, int customer_idx) {
	Customer* c = &g->queue[customer_idx];
	int base_price = 4000; // 기본 음료 가격

	if (c->type == CUST_STUDENT) {
		// 학생: 10% 할인
		g->balance += (base_price * 0.9);
		log_push(g, "학생 할인이 적용되었습니다. (-10%)");
	}
	else if (c->type == CUST_FOODIE) {
		// 미식가: 팁 1500원
		g->balance += (base_price + 1500);
		log_push(g, "미식가가 맛에 감동하여 팁을 주었습니다! (+1500원)");
	}
	else {
		// 직장인: 정가 
		g->balance += base_price;
	}
	c->active = 0; // 서빙 완료 후 퇴장
}


/* 실시간 인내심 차감 및 타임아웃 예외 처리 */
void cust_update(Game* g, Uint32 dt) {
	
	// 손님 스폰 타이머
	cust_spawn(g, dt);

	// 현재 손님들의 인내심 차감
	for (int i = 0; i < MAX_QUEUE; i++) {
		if (g->queue[i].active == 1) {
			g->queue[i].patience_ms -= dt;

			// 인내심이 0이 된 경우 (Timeout)
			if (g->queue[i].patience_ms <= 0) {
				g->queue[i].active = 0; // 손님 퇴장
				g->combo = 0; // 손님이 기다리다 지쳐서 떠나면 연속 콤보 초기화

				log_push(g, "손님이 기다리다 지쳐서 떠났습니다...");
				printf("[WARN] %d번 슬롯 손님 타임아웃 퇴장. 콤보가 초기화됩니다.\n", i+1);
			}
		}
	}
}

/* 큐 인덱스로 손님 데이터 주소 반환 */
Customer* cust_at(Game* g, int qi) { 
	
	int real_idx = (g->q_head + qi) % MAX_QUEUE;

	if (g->queue[real_idx].active) {
		return &g->queue[real_idx];
	}
	return NULL; 
}

/* 제조 슬롯 완료 시 호출될 서빙 */
void cust_serve(Game* g, int slot_idx) {}