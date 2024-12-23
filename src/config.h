#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core.h"
#include "matoya.h"

#define CONFIG_CORE_MAX 64
#define SYSTEM_NAME_MAX 64

#define config_asset_dir()  MTY_JoinPath(MTY_GetProcessDir(), "merton-files")
#define config_path(path)   MTY_JoinPath(config_asset_dir(), path)
#define config_file()       config_path("config.json")
#define config_cores_dir()  config_path("cores")
#define config_save_dir()   config_path("save")
#define config_state_dir()  config_path("state")
#define config_system_dir() config_path("system")
#define config_tmp_dir()    config_path("tmp")
#define config_ui_dir()     config_path("ui")

struct config {
	bool bg_pause;
	bool menu_pause;
	bool console;
	bool fullscreen;
	bool mute;
	bool square_pixels;
	bool int_scaling;
	uint32_t audio_buffer;
	uint32_t playback_rate;
	uint32_t scanlines;
	uint32_t sharpen;
	int32_t vsync;

	MTY_Filter filter;
	MTY_Frame window;

	char core[CORE_SYSTEM_MAX][CONFIG_CORE_MAX];
};
