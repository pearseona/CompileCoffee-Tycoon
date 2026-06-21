#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "common.h"
#include "game.h"
#include "customer.h"
#include "brew.h"
#include "shop.h"
#include "render.h"

int main(int argc, char* argv[]) {
	// SDL 시스템 초기화 (타이머 + 비디오)
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
		printf("SDL 초기화 실패: %s\n", SDL_GetError());
		return -1;
	}

	// SDL_image 초기화 (PNG 로드용)
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		printf("[WARN] SDL_image PNG 초기화 실패: %s\n", IMG_GetError());
	}

	// 폰트 초기화
	if (TTF_Init() < 0) {
		printf("TTF 초기화 실패: %s\n", SDL_GetError());
		IMG_Quit();
		SDL_Quit();
		return -1;
	}

	// 윈도우 창 생성
	SDL_Window* win = SDL_CreateWindow(
		"[Compile Coffee - Cute & Cozy Cafe]",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN
	);

	if (!win) {
		printf("윈도우 생성 실패: %s\n", SDL_GetError());
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!ren) {
		ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
	}

	// 폰트 호출
	if (!render_init_fonts()) {
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return -1;
	}

	if (!render_init_images(ren)) {
		printf("[WARN] 이미지 로드 중 일부 실패가 발생했으나 기본 그래픽으로 진행합니다.\n");
	}

	Game myGame;
	game_init(&myGame);

	// 초기 선택 상태 완벽 클리닝
	myGame.sel_slot = 0;
	myGame.sel_cust = -1;
	myGame.sel_menu = -1;

	// 🎯 [수정]: 시작하자마자 게임을 강제 개시하던 유도 코드를 주석 제거/수정하여 타이틀(STATE_MAIN) 단계에 고정!
	myGame.state = STATE_MAIN;

	printf("[컴파일 커피(Compile Coffee) - 실시간 로그] \n");

	SDL_Delay(500);

	Uint32 lastTime = SDL_GetTicks();
	bool isRunning = true;
	SDL_Event ev;

	while (isRunning) {
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				isRunning = false;
			}
			else if (ev.type == SDL_MOUSEBUTTONDOWN) {
				int mx = ev.button.x;
				int my = ev.button.y;

				/* 🎯 [추가]: 메인 타이틀(STATE_MAIN) 화면 갈색 버튼 클릭 인터랙션 연산 */
				if (myGame.state == STATE_MAIN) {
					// 1. [게임 시작] 갈색 버튼 클릭 체크 (X: 380~580, Y: 340~395)
					if (mx >= 380 && mx <= 580 && my >= 340 && my <= 395) {
						game_start_day(&myGame);
						printf("[INPUT] 타이틀 버튼 클릭 -> 게임 영업 개시!\n");
					}
					// 2. [게임 설명] 갈색 버튼 클릭 체크 (X: 380~580, Y: 420完整475)
					else if (mx >= 380 && mx <= 580 && my >= 420 && my <= 475) {
						myGame.state = STATE_TUTORIAL;
						printf("[INPUT] 타이틀 버튼 클릭 -> 게임 설명 연동 전환\n");
					}
				}
				/* 🎯 [추가]: 게임 설명(STATE_TUTORIAL) 화면 뷰어 전환 */
				else if (myGame.state == STATE_TUTORIAL) {
					// 게임 설명 화면에서는 화면 내 아무 데나 마우스 왼쪽 버튼을 클릭하면 다시 메인으로 컴백!
					myGame.state = STATE_MAIN;
					printf("[INPUT] 설명 가이드 창 종료 -> 메인 화면 복귀\n");
				}
				// 1. 인게임 영업 단계 플레이 클릭 처리
				else if (myGame.state == STATE_PLAYING) {
					bool clicked_slot = false;

					// 머신 제조 슬롯 마우스 클릭 감지
					for (int i = 0; i < myGame.slot_count; i++) {
						int sx = 70 + i * 80;
						int sy = 400;
						if (mx >= sx && mx <= sx + 60 && my >= sy && my <= sy + 80) {
							myGame.sel_slot = i;
							clicked_slot = true;
							printf("[INPUT] 마우스 클릭: 슬롯 %d 선택\n", i + 1);
							log_push(&myGame, "제조 슬롯을 선택했습니다.");
							break;
						}
					}

					// 대기 손님 구역 마우스 클릭 감지 및 서빙 연동
					int spots_x[3] = { 180, 440, 700 };
					for (int i = 0; i < 3; i++) {
						int cx = spots_x[i];
						int cy = 210;
						if (myGame.queue[i].active && mx >= cx && mx <= cx + 135 && my >= cy && my <= cy + 160) {
							myGame.sel_cust = i;
							printf("[INPUT] 마우스 클릭: 손님 %d 선택\n", i + 1);
							log_push(&myGame, "서빙할 손님을 선택했습니다.");

							if (myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
								if (myGame.slots[myGame.sel_slot].state == SLOT_DONE) {
									cust_serve(&myGame, myGame.sel_slot);
								}
							}
							break;
						}
					}

					// 하단 미니 메뉴판 마우스 가상 클릭 패널 범위 보정 수선
					if (!clicked_slot && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.slots[myGame.sel_slot].state == SLOT_EMPTY) {
							for (int i = 0; i < 6; i++) {
								int ix = 330 + i * 56;
								int iy = 436;
								if (mx >= ix && mx <= ix + 45 && my >= iy && my <= iy + 45) {
									int target_qi = (myGame.sel_cust >= 0) ? myGame.sel_cust : 0;

									if (myGame.queue[target_qi].active) {
										if (g_menu[i].unlocked) {
											brew_start(&myGame, myGame.sel_slot, target_qi, (MenuID)i);
										}
										else {
											char lock_msg[80];
											sprintf_s(lock_msg, sizeof(lock_msg), "%s 메뉴는 상점에서 먼저 해금해야 합니다!", g_menu[i].name);
											log_push(&myGame, lock_msg);
										}
									}
									else {
										log_push(&myGame, "⚠️ 음료를 주문한 손님(좌석)을 먼저 선택해 주세요.");
									}
									break;
								}
							}
						}
					}
				}
				// 🎯 2. 상점 정비 단계(STATE_UPGRADE) 마우스 클릭 판정 통합
				else if (myGame.state == STATE_UPGRADE) {
					// 상점 상단 탭 가상 클릭 감지
					if (my >= 130 && my <= 160) {
						if (mx >= 70 && mx <= 250) {
							myGame.shop_page = 0;
							log_push(&myGame, "장비 및 재고 도매 상점 탭으로 이동했습니다.");
						}
						else if (mx >= 260 && mx <= 440) {
							myGame.shop_page = 1;
							log_push(&myGame, "레시피 신메뉴 연구소 탭으로 이동했습니다.");
						}
					}

					int card_y = 235;
					int card_w = 200;
					int card_h = 175;
					int gap = 8;

					int x0 = 70;
					int x1 = x0 + card_w + gap;
					int x2 = x1 + card_w + gap;
					int x3 = x2 + card_w + gap;

					if (myGame.shop_page == 0) {
						// PAGE 0 카드 클릭
						if (mx >= x0 && mx <= x0 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 0);
						}
						else if (mx >= x1 && mx <= x1 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 1);
						}
						else if (mx >= x2 && mx <= x2 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 2);
						}
						else if (mx >= x3 && mx <= x3 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 3);
						}
					}
					else {
						// PAGE 1 카드 클릭 (신메뉴 해금)
						if (mx >= x0 && mx <= x0 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 4); // 바닐라라떼 해금
						}
						else if (mx >= x1 && mx <= x1 + card_w && my >= card_y && my <= card_y + card_h) {
							shop_buy_upgrade(&myGame, 5); // 콜드브루 해금
						}
					}
				}
			}
			else if (ev.type == SDL_KEYDOWN) {
				int target_qi = (myGame.sel_cust >= 0) ? myGame.sel_cust : 0;

				switch (ev.key.keysym.sym) {
				case SDLK_1:
					if (myGame.state == STATE_PLAYING) {
						myGame.sel_slot = 0;
						log_push(&myGame, "제조 슬롯 1을 선택했습니다.");
					}
					break;
				case SDLK_2:
					if (myGame.state == STATE_PLAYING) {
						if (myGame.slot_count >= 2) {
							myGame.sel_slot = 1;
							log_push(&myGame, "제조 슬롯 2를 선택했습니다.");
						}
						else {
							log_push(&myGame, "상점에서 슬롯을 먼저 확장해 주세요!");
						}
					}
					break;
				case SDLK_3:
					if (myGame.state == STATE_PLAYING) {
						if (myGame.slot_count >= 3) {
							myGame.sel_slot = 2;
							log_push(&myGame, "제조 슬롯 3을 선택했습니다.");
						}
						else {
							log_push(&myGame, "상점에서 슬롯을 먼저 확장해 주세요!");
						}
					}
					break;
				case SDLK_8:
					if (myGame.state == STATE_UPGRADE && myGame.shop_page == 0) shop_buy_upgrade(&myGame, 0);
					break;
				case SDLK_9:
					if (myGame.state == STATE_UPGRADE && myGame.shop_page == 0) shop_buy_upgrade(&myGame, 1);
					break;
				case SDLK_LEFT:
					if (myGame.state == STATE_UPGRADE) {
						myGame.shop_page = 0;
						log_push(&myGame, "장비 및 재고 도매 상점 탭으로 이동했습니다.");
					}
					break;
				case SDLK_RIGHT:
					if (myGame.state == STATE_UPGRADE) {
						myGame.shop_page = 1;
						log_push(&myGame, "레시피 신메뉴 연구소 탭으로 이동했습니다.");
					}
					break;

				case SDLK_q:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_AMERICANO].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_AMERICANO);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;
				case SDLK_w:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_LATTE].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_LATTE);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;
				case SDLK_e:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_VANILLA_LATTE].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_VANILLA_LATTE);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;
				case SDLK_r:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_COLD_BREW].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_COLD_BREW);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;
				case SDLK_t:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_CARAMEL_MAC].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_CARAMEL_MAC);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;
				case SDLK_y:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.queue[target_qi].active && g_menu[MENU_ESPRESSO].unlocked) {
							brew_start(&myGame, myGame.sel_slot, target_qi, MENU_ESPRESSO);
						}
						else if (!myGame.queue[target_qi].active) {
							log_push(&myGame, "⚠️ 해당 자리에 음료를 주문한 손님이 없습니다.");
						}
					}
					break;

				case SDLK_a:
					if (myGame.state == STATE_PLAYING && myGame.queue[0].active) myGame.sel_cust = 0;
					break;
				case SDLK_s:
					if (myGame.state == STATE_PLAYING && myGame.queue[1].active) myGame.sel_cust = 1;
					break;
				case SDLK_d:
					if (myGame.state == STATE_PLAYING && myGame.queue[2].active) myGame.sel_cust = 2;
					break;
				case SDLK_SPACE:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) cust_serve(&myGame, myGame.sel_slot);
					break;
				case SDLK_c:
				case SDLK_BACKSPACE:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) brew_cancel(&myGame, myGame.sel_slot);
					break;
				case SDLK_ESCAPE:
					isRunning = false;
					break;
				case SDLK_RETURN:
					if (myGame.state == STATE_UPGRADE) {
						shop_next_day(&myGame);
						myGame.sel_slot = 0;
						myGame.sel_cust = -1;
					}
					break;
				}
			}
		}

		Uint32 currentTime = SDL_GetTicks();
		Uint32 dt = currentTime - lastTime;

		if (dt >= FRAME_DELAY) {
			// 게임 가동 중(PLAYING)일 때만 시간을 정상 흐르게 예외 제어
			if (myGame.state == STATE_PLAYING) {
				game_update(&myGame, dt);
			}
			render_frame(ren, &myGame);
			lastTime = currentTime;
		}
		SDL_Delay(1);
	}

	render_close_fonts();
	render_close_images();
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}