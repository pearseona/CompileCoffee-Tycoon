#include <string.h>
#include "common.h"
#include "game.h"

/* 상점 업그레이드 및 소모품 상품 목록 초기화 */
void upg_init(Game* g) {

	// [인덱스 0] 제조 슬롯 확장 
	strcpy_s(g->upg[0].name, sizeof(g->upg[0].name), "제조 슬롯 확장");
	strcpy_s(g->upg[0].desc, sizeof(g->upg[0].desc), "동시에 음료를 제조할 수 있는 슬롯을 추가합니다.");
	g->upg[0].base_cost = 3000;
	g->upg[0].level = 1;
	g->upg[0].max_level = MAX_BREW_SLOTS;

	// [인덱스 1] 머신 속도 향상 
	strcpy_s(g->upg[1].name, sizeof(g->upg[1].name), "머신 속도 향상");
	strcpy_s(g->upg[1].desc, sizeof(g->upg[1].desc), "음료 제조 속도가 15% 빨라집니다.");
	g->upg[1].base_cost = 25000;
	g->upg[1].level = 0;
	g->upg[1].max_level = 3;

	// [인덱스 2] 원두 구매 
	strcpy_s(g->upg[2].name, sizeof(g->upg[2].name), "원두 자루(x10)");
	strcpy_s(g->upg[2].desc, sizeof(g->upg[2].desc), "원두 재고를 도매가로 10개 대량 확보합니다.");
	g->upg[2].base_cost = 1500;
	g->upg[2].level = 0;
	g->upg[2].max_level = 999;

	// [인덱스 3] 우유 구매 
	strcpy_s(g->upg[3].name, sizeof(g->upg[3].name), "우유 팩 묶음(x10)");
	strcpy_s(g->upg[3].desc, sizeof(g->upg[3].desc), "우유 재고를 도매가로 10개 대량 확보합니다.");
	g->upg[3].base_cost = 1000;
	g->upg[3].level = 0;
	g->upg[3].max_level = 999;

	// [인덱스 4] 레시피 해금: 바닐라 라떼
	strcpy_s(g->upg[4].name, sizeof(g->upg[4].name), "바닐라라떼 해금");
	strcpy_s(g->upg[4].desc, sizeof(g->upg[4].desc), "시럽이 들어간 달콤한 바닐라라떼 메뉴를 오픈합니다.");
	g->upg[4].base_cost = 6000;
	g->upg[4].level = 0;
	g->upg[4].max_level = 1;

	// [인덱스 5] 레시피 해금: 콜드 브루
	strcpy_s(g->upg[5].name, sizeof(g->upg[5].name), "콜드브루 해금");
	strcpy_s(g->upg[5].desc, sizeof(g->upg[5].desc), "얼음이 한가득 들어간 고급 깔끔한 콜드브루를 해금합니다.");
	g->upg[5].base_cost = 9000;
	g->upg[5].level = 0;
	g->upg[5].max_level = 1;
}

void upg_but(Game* g, int idx) {
	// 필요 시 래퍼 함수로 활용 가능
}