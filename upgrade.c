// 업그레이드 초기화 및 구매 관리 모듈
#include <string.h>
#include "common.h"
#include "game.h"

/* 상점 업그레이드 및 소모품 상품 목록 초기화 */
void upg_init(Game* g) {
	// [인덱스 0] 제조 슬롯 확장 스펙 셋업
	strcpy_s(g->upg[0].name, sizeof(g->upg[0].name), "제조 슬롯 확장");
	strcpy_s(g->upg[0].desc, sizeof(g->upg[0].desc), "동시에 음료를 제조할 수 있는 슬롯을 추가합니다.");
	g->upg[0].base_cost = 3000;
	g->upg[0].level = 1;
	g->upg[0].max_level = MAX_BREW_SLOTS;

	// [인덱스 1] 머신 속도 향상 스펙 셋업
	strcpy_s(g->upg[1].name, sizeof(g->upg[1].name), "머신 속도 향상");
	strcpy_s(g->upg[1].desc, sizeof(g->upg[1].desc), "음료 제조 속도가 15% 빨라집니다.");
	g->upg[1].base_cost = 25000;
	g->upg[1].level = 0;
	g->upg[1].max_level = 3;

	// 📦 [인덱스 2] 대안 B: 원두 대량 도매 구매 상품 추가
	strcpy_s(g->upg[2].name, sizeof(g->upg[2].name), "원두 자루(x10)");
	strcpy_s(g->upg[2].desc, sizeof(g->upg[2].desc), "원두 재고를 도매가로 10개 대량 확보합니다.");
	g->upg[2].base_cost = 1500; // 도매가라 10개에 1500원 (인게임 긴급 충전보다 훨씬 저렴!)
	g->upg[2].level = 0;
	g->upg[2].max_level = 999; // 소모품이므로 만렙 제한을 사실상 무제한으로 설정

	// 📦 [인덱스 3] 대안 B: 우유 대량 도매 구매 상품 추가
	strcpy_s(g->upg[3].name, sizeof(g->upg[3].name), "우유 팩 묶음(x10)");
	strcpy_s(g->upg[3].desc, sizeof(g->upg[3].desc), "우유 재고를 도매가로 10개 대량 확보합니다.");
	g->upg[3].base_cost = 1000; // 도매가 10개에 1000원
	g->upg[3].level = 0;
	g->upg[3].max_level = 999;
}

// 기존 shop.c의 shop_buy_upgrade 함수를 사용하므로 이 함수는 비워두거나 제외해도 무방합니다.
void upg_but(Game* g, int idx) {
	// 필요 시 shop_buy_upgrade(g, idx)를 호출하는 래퍼 함수로 쓸 수 있습니다.
}