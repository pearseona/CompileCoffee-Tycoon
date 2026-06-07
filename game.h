#pragma once
#ifndef GAME_H
#define GAME_H

#include "./common.h"

void game_init(Game* g);
void game_start_day(Game* g);
void game_close_day(Game* g);
void game_update(Game* g, Uint32 dt);
void log_push(Game* g, const char* msg);
int clamp_i(int v, int lo, int hi);

#endif