#pragma once

#include "matoya.h"

Core *rcore_load(MTY_SO *so, const char *system_dir);
void rcore_unload(Core **core);
bool rcore_load_game(Core *ctx, CoreSystem system, const char *path,
	const void *save_data, size_t save_data_size);
void rcore_unload_game(Core *ctx);
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
const CoreSetting *rcore_get_all_settings(Core *ctx, uint32_t *len);
void rcore_set_setting(Core *ctx, const char *key, const char *val);
const char *rcore_get_setting(Core *ctx, const char *key);
void rcore_reset_settings(Core *ctx);
