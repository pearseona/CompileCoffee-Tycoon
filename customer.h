#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "common.h"

// 함수 원형 선언
void cust_init(Game* g);
void cust_spawn(Game* g, Uint32 dt);
void cust_update(Game* g, Uint32 dt);
void game_serve_drink(Game* g, int customer_idx);
Customer* cust_at(Game* g, int qi);
void cust_serve(Game* g, int slot_idx);

#endif // CUSTOMER_H