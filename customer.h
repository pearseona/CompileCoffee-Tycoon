#pragma once

#include "common.h"

#define SPAWN_INTERVAL_MS 7000 // 7초마다 손님 입장

// 대기열 및 타이머 초기화
void cust_init(Game* g);

// 델타 타임 누적 및 조건별 손님 생성
void cust_spawn(Game* g, Uint32 dt);

// 지정된 큐 인덱스의 손님 정산 및 퇴장 처리
void game_serve_drink(Game* g, int customer_idx);

// 실시간 인내심 차감 및 타임아웃 예외 처리
void cust_update(Game* g, Uint32 dt);

// 큐 인덱스로 손님 데이터 주소 반환
Customer* cust_at(Game* g, int qi);

// 제조 슬롯 완료 시 호출될 서빙
void cust_serve(Game* g, int slot_idx);
