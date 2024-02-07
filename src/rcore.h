#pragma once

#include "matoya.h"

void rcore_set_so(MTY_SO *so);
Core *rcore_load_game(CoreSystem system, const char *system_dir, const char *path,
	const void *save_data, size_t save_data_size);
void rcore_unload_game(Core **core);
void rcore_reset(Core *ctx);
void rcore_run(Core *ctx);
void rcore_set_button(Core *ctx, uint8_t player, CoreButton button, bool pressed);
void rcore_set_axis(Core *ctx, uint8_t player, CoreAxis axis, int16_t value);
void *rcore_get_state(Core *ctx, size_t *size);
bool rcore_set_state(Core *ctx, const void *state, size_t size);
bool rcore_insert_disc(Core *ctx, const char *path);
void *rcore_get_save_data(Core *ctx, size_t *size);
double rcore_get_frame_rate(Core *ctx);
float rcore_get_aspect_ratio(Core *ctx);
void rcore_set_log_func(Core *ctx, CoreLogFunc func, void *opaque);
void rcore_set_audio_func(Core *ctx, CoreAudioFunc func, void *opaque);
void rcore_set_video_func(Core *ctx, CoreVideoFunc func, void *opaque);
CoreSetting *rcore_get_settings(uint32_t *len);
void rcore_update_settings(Core *ctx);
