// 공통 데이터 정의 묘듈
#ifndef COMMON_H
#define COMMON_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*  화면 & 프레임 세팅 */
#define SCREEN_W		960
#define SCREEN_H        620
#define FPS             60
#define FRAME_DELAY     (1000 / FPS)

/* 게임 상수 */
#define MAX_QUEUE 8 // 손님 대기열
#define MAX_BREW_SLOTS 3 // 업그레이드 가능한 제조 슬롯
#define MAX_MENU 6 // 아메리카노, 라떼 등 메뉴 6종
#define MAX_INGREDIENT 5 // 원두, 우유, 시럽, 크림, 얼음
#define MAX_DAYS 30 // 총 플레이 타임라인 30일
#define DAY_SEC 90 // 하루 영업 시간; 실시간 90초
#define COMBO_TIMEOUT_MS 7000 // 콤보 유지 시간 제한 (7초)
#define COMBO_COOL_DOWN_MS 5000 // 콤보 잠금 해제까지 걸리는 시간 (5초)
#define LOG_MAX 6 // 인게임 HUD에 노출할 최근 로그 수
#define MAX_LOG_LINES LOG_MAX
#define GOAL_PROFIT 1500000 // 최종 승리 조건: 누적 순이익 150만원

/* 열거형(Enum) 정의*/

// 게임의 전체 흐름 제어 
typedef enum {
    STATE_MAIN = 0,
    STATE_TUTORIAL,
    STATE_PLAYING,
    STATE_CLOSING,
    STATE_UPGRADE,
    STATE_GAMEOVER,
    STATE_HIGHSCORE,
    STATE_QUIT
} GameState;

// 메뉴 ID
typedef enum {
    MENU_AMERICANO = 0,
    MENU_LATTE,
    MENU_VANILLA_LATTE,
    MENU_COLD_BREW,
    MENU_CARAMEL_MAC,
    MENU_ESPRESSO
} MenuID;

// 재료 ID
typedef enum {
    ING_BEAN = 0,
    ING_MILK,
    ING_SYRUP,
    ING_CREAM,
    ING_ICE
} IngID;

// 손님 성향 유형
typedef enum {
    CUST_WORKER = 0, // 직장인: 성격 급함
    CUST_FOODIE, // 미식가: 고급 메뉴 요구, 높은 팁
    CUST_STUDENT, // 학생: 할인 적용

    CUST_TYPE_COUNT

}CustType;

// 제조 슬롯의 실시간 상태
typedef enum {
    SLOT_EMPTY = 0,
    SLOT_BREWING,
    SLOT_DONE
} SlotState;

/* 구조체 (Struct) 정의 */

// 메뉴 정보 테이블 구조체
typedef struct {
    const char* name;
    int price;
    int cost;
    int brew_ms;
    int ing[MAX_INGREDIENT];
    int unlocked;
} MenuInfo;

// 손님 구조체
typedef struct {
    int id;
    CustType type;
    MenuID order;
    int patience_ms; // 실시간 감소할 인내심 수치
    int patience_max;
    int active;
    int served; // 상태 정산: 1=만족 서빙 완료, -1=기다리다 이탈
}Customer;

// 음료 제조 슬롯 구조체
typedef struct {
    SlotState state;
    MenuID menu;
    int cust_id;
    int elapsed_ms; // 현재 제조 진행 시간
    int required_ms; // 메뉴별 완료 필요 시간
}BrewSlot;

// 업그레이드 아이템 구조체
typedef struct {
    char name[40];
    char desc[80];
    int  base_cost;
    int  level;
    int  max_level;
} Upgrade;

// 일별 재정/운영 기록 데이터 구조체 (파일 I/O)
typedef struct {
    int day, revenue, expenditure, profit;
    int served, left, max_combo;
} DayRecord;

// 게임 통합 구조체
typedef struct {
    GameState state;

    // 경제 및 평판
    int day, balance, reputation;
    int day_revenue, day_expenditure, total_profit;

    // 실시간 타이머 및 스폰 시스템
    int day_ms;
    Uint32 last_tick;
    int spawn_timer_ms;

    // 큐 관리
    Customer queue[MAX_QUEUE];
    int q_head, q_tail, q_size, next_id;

    // 워크스테이션 제조 슬롯
    BrewSlot slots[MAX_BREW_SLOTS];
    int slot_count;

    // 원자재 재고 창고
    int stock[MAX_INGREDIENT];

    int stock_refill_ms[MAX_INGREDIENT];
    int is_refilling[MAX_INGREDIENT];

    // 콤보 시스템
    int combo, max_combo;
    Uint32 combo_timer;
    Uint32 combo_lock_time;

    // 업그레이드 트리
    Upgrade upg[6];

    // 영속 저쟝용 데이터 배열
    DayRecord records[MAX_DAYS];
    int total_served, total_left;

    // 인터랙션 유선 UI 선택 상태 값
    int sel_slot, sel_cust, sel_menu;
    int show_recipe;

    // 이벤트 NPS 관련 데이터
    int inspector_day, influencer_day;
    char event_msg[128];
    Uint32 event_ms;

    // 튜토리얼 시스템
    int tutorial_page;

    // 상점 페이지 내비게이션 상태 변수
    int shop_page;

    // 화면 HUD 실시간 로그 피드백
    char log_lines[LOG_MAX][80];
    Uint32 log_ttl[LOG_MAX];
    int log_count;

    // 애니메이션 동기화용 틱
    Uint32 anim_tick;

} Game;

/* 전역 데이터 선언 */
extern MenuInfo g_menu[MAX_MENU];
extern const char* g_ing_name[MAX_INGREDIENT];
extern const char* g_cust_name[3];

/* 프로토타입 선언 */

// game.c
void game_init(Game* g);
void game_start_day(Game* g);
void game_close_day(Game* g);
void game_update(Game* g, Uint32 dt);

// customer.c
void cust_spawn(Game* g, Uint32 dt);
void cust_update(Game* g, Uint32 dt);
Customer* cust_at(Game* g, int qi);
void cust_serve(Game* g, int slot_idx);

// brew.c
void brew_start(Game* g, int slot_idx, int qi, MenuID menu);
void brew_update(Game* g, Uint32 dt);
void brew_cancel(Game* g, int slot_idx);

// upgrade.c
void upg_init(Game* g);
// void upg_buy(Game* g, int idx);

// save.c
void save_record(Game* g);
int load_highscore(void);
int save_load_game(Game* g);

// render.c
void render_frame(SDL_Renderer* ren, Game* g);

void log_push(Game* g, const char* msg);
int clamp_i(int v, int lo, int hi);

#endif



