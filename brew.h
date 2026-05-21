#pragma once

#include "common.h"

// 특정 제조 슬롯에서 음료 제조를 개시
void brew_start(Game* g, int slot_idx, int qi, MenuID menu);

// 제조 진행 시간을 실시간 누적
void brew_update(Game* g, Uint32 dt);

// 제조 중인 슬롯 강제 취소 (원자재는 버려짐)
void brew_cancel(Game* g, int slot_idx);
