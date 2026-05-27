#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "common.h"
#include "game.h"     
#include "customer.h"
#include "brew.h"
#include "shop.h"

int main(int argc, char* argv[]) {

	// SDL 시스템 초기화 (타이머 + 비디오)
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
		printf("SDL 초기화 실패: %s\n", SDL_GetError());
		return -1;
	}

	// 폰트 초기화
	if (TTF_Init() < 0) {
		printf("TTF 초기화 실패: %s\n", TTF_GetError());
		return -1;
	}

	// 윈도우 창 생성
	SDL_Window* win = SDL_CreateWindow(
		"[Compile Coffee]",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN
	);

	if (!win) {
		printf("윈도우 생성 실패: %s\n", SDL_GetError());
		TTF_Quit();
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

	Game myGame;
	game_init(&myGame); // 첫 자본금 및 리시피 세팅
	game_start_day(&myGame); // Day 1 영업 강제 개시

	printf("[컴파일 커피(Compile Coffee) - 실시간 로그] \n");

	SDL_Delay(1000); // 가동 전 1초 대기

	Uint32 lastTime = SDL_GetTicks(); // 이전 프레임의 시간 기록
	bool isRunning = true;

	SDL_Event ev;

	// 메인 게임 루프
	while (isRunning) {

		// 창 닫기 버튼(X)을 누르면 안전하게 종료
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				isRunning = false;
			}

			// 키보드 입력 이벤트 처리
			else if (ev.type == SDL_KEYDOWN) {
				switch (ev.key.keysym.sym) {

				case SDLK_1: // 1: [PLAYING] 음료 서빙 / [CLOSING] 상점 이동
					if (myGame.state == STATE_PLAYING) {
						cust_serve(&myGame, 0);
					}
					else if (myGame.state == STATE_CLOSING) {
						myGame.state = STATE_UPGRADE;
						log_push(&myGame, "상점에 입장했습니다. 필요한 업그레이드를 진행하세요.");
						printf("[STATE] 마감 정산 확인 완료 -> 상점 화면(STATE_UPGRADE) 진입\n");
					}
					break;


				/* ===== [상점 전용 입력] 상점 관련(STATE_UPGRADE) ===== */

				case SDLK_8: // 8: 제조 슬롯 확장 구매 시도
					if (myGame.state == STATE_UPGRADE) {
						shop_buy_upgrade(&myGame, 0);
					}
					break;

				case SDLK_9: // 9: 머신 속도 향상 구매 시도
					if (myGame.state == STATE_UPGRADE) {
						shop_buy_upgrade(&myGame, 1);
					}
					break;

				case SDLK_RETURN: // Enter: 다음 날(Day++) 영업 개시
					if (myGame.state == STATE_UPGRADE) {
						shop_next_day(&myGame);
					}
					break;

				/* ===== 인게임 플레이 제조 키 ===== */

				case SDLK_2: // 2: 아메리카노 제조
					if (myGame.state == STATE_PLAYING) {
						// 초기 해금(1) 상태
						if (g_menu[MENU_AMERICANO].unlocked) {
							brew_start(&myGame, 0, 0, MENU_AMERICANO);
						}
						else {
							log_push(&myGame, "아직 오픈되지 않은 메뉴입니다!");
						}
					}
					break;

				case SDLK_3: // 3: 카페라떼 제조
					if (myGame.state == STATE_PLAYING) {
						// 초기 해금(1) 상태
						if (g_menu[MENU_LATTE].unlocked) {
							brew_start(&myGame, 0, 0, MENU_LATTE);
						}
						else {
							log_push(&myGame, "아직 오픈되지 않은 메뉴입니다!");
						}

					}
					break;

				case SDLK_4: // 4: 바닐라라뗴 제조(초기 잠금)
					if (myGame.state == STATE_PLAYING) {
						// unlocked이 1이 되어야만 작동
						if (g_menu[MENU_VANILLA_LATTE].unlocked) {
							brew_start(&myGame, 0, 0, MENU_VANILLA_LATTE);
						}
						else {
							log_push(&myGame, "바닐라라떼는 상점에서 먼저 오픈해야 합니다!");
							printf("[LOCK] 미오픈 메뉴 접근 차단: 바닐라라떼\n");
						}
					}
					break;

				case SDLK_5: // 5: 콜드브루 제조 (초기 잠금)
					if (myGame.state == STATE_PLAYING) {
						if (g_menu[MENU_COLD_BREW].unlocked) {
							brew_start(&myGame, 0, 0, MENU_COLD_BREW);
						}
						else {
							log_push(&myGame, "콜드브루는 상점에서 먼저 오픈해야 합니다!");
							printf("[LOCK] 미오픈 메뉴 접근 차단: 콜드브루\n");
						}
					}
					break;

				case SDLK_6: // 6: 카라멜 마키아토 제조 (초기 잠금)
					if (myGame.state == STATE_PLAYING) {
						if (g_menu[MENU_CARAMEL_MAC].unlocked) {
							brew_start(&myGame, 0, 0, MENU_CARAMEL_MAC);
						}
						else {
							log_push(&myGame, "카라멜 마키아토는 상점에서 먼저 오픈해야 합니다!");
							printf("[LOCK] 미오픈 메뉴 접근 차단: 카라멜 마키아토\n");
						}
					}
					break;

				case SDLK_7: // 7: 에스프레소 제조
					if (myGame.state == STATE_PLAYING) {
						// 초기 해금(1) 상태
						if (g_menu[MENU_ESPRESSO].unlocked) {
							brew_start(&myGame, 0, 0, MENU_ESPRESSO);
						}
						else {
							log_push(&myGame, "아직 오픈되지 않은 메뉴입니다!");
						}
					}
					break;

				case SDLK_ESCAPE: // ESC 누르면 즉시 종료
					isRunning = false;
					break;
				}
			}
		}

		Uint32 currentTime = SDL_GetTicks();
		Uint32 dt = currentTime - lastTime; // 이전 프레임과 현재 프레임 사이의 시간 차이

		if (dt >= FRAME_DELAY) {

			// 게임 내부 타이머 및 시스템 업데이트
			game_update(&myGame, dt);

			// 임시 화면 출력
			render_frame(ren, NULL, NULL, NULL, &myGame);

			// 만약 시간이 다 되어서 마감 상태로 넘어가면 하루 루프 종료
			if (myGame.state == STATE_CLOSING) {

				static int last_printed_day = 0;

				if (last_printed_day != myGame.day) {
					printf("\n========================================\n");
					printf(" %d일차 영업이 무사히 마감되었습니다!\n", myGame.day);
					printf(" 오늘 하루 총 수입: %d원\n ", myGame.day_revenue);
					printf(" 현재 총 잔액: %d원\n", myGame.balance);
					printf(" -> [1]번 키를 누르면 상점 화면으로 이동합니다.\n");
					printf("=======================================\n\n");
					last_printed_day = myGame.day;
				}

			}

			else if (myGame.state == STATE_UPGRADE) {

				static int shop_announce_day = 0;

				if (shop_announce_day != myGame.day) {
					printf("\n========================================\n");
					printf(" [SHOP] 제 %d일차 상점 정비 페이즈에 진입했습니다.\n", myGame.day);
					printf(" 현재 보유 자본금: %d원 | 현재 제조 슬롯 수: %d개\n", myGame.balance, myGame.slot_count);
					printf(" ----------------------------------------\n");
					printf("  [8] 제조 슬롯 확장 구매 (비용: 3000원)\n");
					printf("  [9] 머신 속도 향상 구매 (비용: 25000원)\n");
					printf(" ----------------------------------------\n");
					printf(" ➔ 업그레이드를 마쳤다면 [Enter]를 눌러 다음 날 영업을 시작하세요.\n");
					printf("=======================================\n\n");
					shop_announce_day = myGame.day;
				}
			}

			lastTime = currentTime; // 시간 갱신
		}
		
		SDL_Delay(1);
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	TTF_Quit();
	SDL_Quit();
	return 0;
}