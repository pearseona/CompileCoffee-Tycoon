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

	game_start_day(&myGame);

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

				if (myGame.state == STATE_PLAYING) {
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

					for (int i = 0; i < 3; i++) {
						int cx = 180 + i * 260;
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

					if (!clicked_slot && myGame.sel_slot >= 0 && myGame.sel_slot < myGame.slot_count) {
						if (myGame.slots[myGame.sel_slot].state == SLOT_EMPTY) {
							for (int i = 0; i < 6; i++) {
								int ix = 330 + i * 55;
								int iy = 425;
								if (mx >= ix && mx <= ix + 45 && my >= iy && my <= iy + 45) {
									if (g_menu[i].unlocked) {
										brew_start(&myGame, myGame.sel_slot, 0, (MenuID)i);
									}
									else {
										char lock_msg[80];
										sprintf_s(lock_msg, sizeof(lock_msg), "%s 메뉴는 상점에서 먼저 해금해야 합니다!", g_menu[i].name);
										log_push(&myGame, lock_msg);
									}
									break;
								}
							}
						}
					}
				}
			}
			else if (ev.type == SDL_KEYDOWN) {
				switch (ev.key.keysym.sym) {
				case SDLK_1:
					if (myGame.state == STATE_PLAYING) {
						myGame.sel_slot = 0;
						log_push(&myGame, "제조 슬롯 1을 선택했습니다.");
					}
					else if (myGame.state == STATE_CLOSING) {
						myGame.state = STATE_UPGRADE;
						log_push(&myGame, "상점에 입장했습니다. 필요한 업그레이드를 진행하세요.");
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
					if (myGame.state == STATE_UPGRADE) {
						shop_buy_upgrade(&myGame, 0);
					}
					break;
				case SDLK_9:
					if (myGame.state == STATE_UPGRADE) {
						shop_buy_upgrade(&myGame, 1);
					}
					break;
				case SDLK_RETURN:
					if (myGame.state == STATE_UPGRADE) {
						shop_next_day(&myGame);
						myGame.sel_slot = 0;
						myGame.sel_cust = -1;
					}
					break;
				case SDLK_q:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_AMERICANO].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_AMERICANO);
					}
					break;
				case SDLK_w:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_LATTE].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_LATTE);
					}
					break;
				case SDLK_e:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_VANILLA_LATTE].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_VANILLA_LATTE);
					}
					break;
				case SDLK_r:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_COLD_BREW].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_COLD_BREW);
					}
					break;
				case SDLK_t:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_CARAMEL_MAC].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_CARAMEL_MAC);
					}
					break;
				case SDLK_y:
					if (myGame.state == STATE_PLAYING && myGame.sel_slot >= 0) {
						if (g_menu[MENU_ESPRESSO].unlocked) brew_start(&myGame, myGame.sel_slot, 0, MENU_ESPRESSO);
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
				}
			}
		}

		Uint32 currentTime = SDL_GetTicks();
		Uint32 dt = currentTime - lastTime;

		if (dt >= FRAME_DELAY) {
			game_update(&myGame, dt);
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