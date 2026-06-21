#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 
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
	// 🎯 [난수 버그 완치]: 프로그램 구동 직후 시간 기반 시드를 초기화하여 무한 학생 스폰 억까를 전격 해결!
	srand((unsigned int)time(NULL));

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
		printf("SDL 초기화 실패: %s\n", SDL_GetError());
		return -1;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		printf("[WARN] SDL_image PNG 초기화 실패: %s\n", IMG_GetError());
	}

	if (TTF_Init() < 0) {
		printf("TTF 초기화 실패: %s\n", SDL_GetError());
		IMG_Quit();
		SDL_Quit();
		return -1;
	}

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

	myGame.sel_slot = 0;
	myGame.sel_cust = -1;
	myGame.sel_menu = -1;
	myGame.is_paused = 0;
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

				if (myGame.state == STATE_MAIN) {
					if (mx >= 380 && mx <= 580 && my >= 340 && my <= 395) {
						game_start_day(&myGame);
						myGame.is_paused = 0;
						printf("[INPUT] 타이틀 버튼 클릭 -> 게임 영업 개시!\n");
					}
					else if (mx >= 380 && mx <= 580 && my >= 420 && my <= 475) {
						myGame.state = STATE_TUTORIAL;
						printf("[INPUT] 타이틀 버튼 클릭 -> 게임 설명 연동 전환\n");
					}
				}
				else if (myGame.state == STATE_TUTORIAL) {
					myGame.state = STATE_MAIN;
					printf("[INPUT] 설명 가이드 창 종료 -> 메인 화면 복귀\n");
				}
				else if (myGame.state == STATE_PLAYING) {
					if (mx >= 835 && mx <= 887 && my >= 20 && my <= 46) {
						myGame.is_paused = !myGame.is_paused;
						log_push(&myGame, myGame.is_paused ? "⏸️ 게임을 일시 정지했습니다." : "▶ 장사를 재개합니다.");
						continue;
					}
					if (mx >= 892 && mx <= 924 && my >= 20 && my <= 46) {
						myGame.state = STATE_MAIN;
						myGame.is_paused = 0;
						log_push(&myGame, "🏠 홈 타이틀 화면으로 돌아왔습니다.");
						continue;
					}

					if (myGame.is_paused) {
						log_push(&myGame, "⏸️ 일시정지 중에는 행동할 수 없다냥! 다시 눌러 해제하라냥.");
						continue;
					}

					bool clicked_slot = false;

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

					int spots_x[3] = { 180, 440, 700 };
					for (int i = 0; i < 3; i++) {
						int cx = spots_x[i];
						int cy = 165;

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
				else if (myGame.state == STATE_UPGRADE) {
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

					int gap = 8; int card_w = 175; int card_h = 160;
					int x0 = 65;
					int x1 = x0 + card_w + gap;
					int x2 = x1 + card_w + gap;
					int x3 = x2 + card_w + gap;

					if (myGame.shop_page == 0) {
						if (my >= 230 && my <= 230 + card_h) {
							if (mx >= x0 && mx <= x0 + card_w) shop_buy_upgrade(&myGame, 0);
							else if (mx >= x1 && mx <= x1 + card_w) shop_buy_upgrade(&myGame, 1);
							else if (mx >= x2 && mx <= x2 + card_w) shop_buy_upgrade(&myGame, 2);
							else if (mx >= x3 && mx <= x3 + card_w) shop_buy_upgrade(&myGame, 3);
						}
						else if (my >= 400 && my <= 510) {
							if (mx >= x0 && mx <= x0 + card_w) shop_buy_upgrade(&myGame, 6);
							else if (mx >= x1 && mx <= x1 + card_w) shop_buy_upgrade(&myGame, 7);
							else if (mx >= x2 && mx <= x2 + card_w) shop_buy_upgrade(&myGame, 8);
						}
					}
					else {
						int big_card_w = 230; int big_card_h = 175; int big_gap = 15;
						int rx0 = x0;
						int rx1 = rx0 + big_card_w + big_gap;
						int rx2 = rx1 + big_card_w + big_gap;

						if (my >= 235 && my <= 235 + big_card_h) {
							if (mx >= rx0 && mx <= rx0 + big_card_w) shop_buy_upgrade(&myGame, 4);
							else if (mx >= rx1 && mx <= rx1 + big_card_w) shop_buy_upgrade(&myGame, 5);
							else if (mx >= rx2 && mx <= rx2 + big_card_w) shop_buy_upgrade(&myGame, 9);
						}
					}
				}
				/* ================= 🎯 [추가 기획]: 우승 및 실패 엔딩창 마우스 버튼 연동 인터랙션 ================= */
				else if (myGame.state == STATE_HIGHSCORE || myGame.state == STATE_GAMEOVER) {
					// 1. [🔄 다시 하기] 버튼 조작 감지 (X: 250 ~ 430, Y: 390 ~ 440)
					if (mx >= 250 && mx <= 430 && my >= 390 && my <= 440) {
						game_init(&myGame);
						game_start_day(&myGame); // 초기화 후 바로 1일차 영업 시작
						myGame.is_paused = 0;
						printf("[INPUT] 엔딩 창 -> 다시 하기 선택 완료! 장사 리스타트.\n");
					}
					// 2. [🏠 홈으로] 버튼 조작 감지 (X: 530 ~ 710, Y: 390 ~ 440)
					else if (mx >= 530 && mx <= 710 && my >= 390 && my <= 440) {
						game_init(&myGame);
						myGame.state = STATE_MAIN; // 타이틀 메인 화면 원위치
						printf("[INPUT] 엔딩 창 -> 타이틀 홈 화면 복귀.\n");
					}
				}
			}
			else if (ev.type == SDL_KEYDOWN) {
				if (myGame.state == STATE_PLAYING && myGame.is_paused) {
					continue;
				}

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
				case SDLK_p:
					if (myGame.state == STATE_PLAYING) {
						myGame.is_paused = !myGame.is_paused;
					}
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
			if (myGame.state == STATE_PLAYING && !myGame.is_paused) {
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