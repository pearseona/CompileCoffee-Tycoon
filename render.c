// SDL2 전체 렌더링
#include <stdio.h>
#include "common.h"

void render_frame(SDL_Render* ren, TTF_Font* fnt_lg, TTF_Font* fnt_md, TTF_Font* fnt_sm, Game* g) {

	system("cls");
    printf("==================================================\n");
    printf(" ☕ 컴파일 커피 (Compile Coffee) 실시간 모니터링 \n");
    printf("==================================================\n");
    printf(" [Day %d] 잔액: %d원 | 평판: %d점 | 콤보: %d🔥\n", g->day, g->balance, g->reputation, g->combo);
    printf(" 남은 영업 시간: %d초\n", g->day_ms / 1000);
    printf("==================================================\n");
    printf("보유 재고 -> 원두: %d | 우유: %d | 시럽: %d\n", g->stock[ING_BEAN], g->stock[ING_MILK], g->stock[ING_SYRUP]);
    printf("--------------------------------------------------\n");
    printf(" [최근 알림 로그]\n");
    for (int i = 0; i < g->log_count; i++) {
        printf(" └ %s\n", g->log_lines[i]);
    }
    printf("==================================================\n");
}