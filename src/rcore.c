#include "core.h"
#include "rcore.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "deps/libretro.h"

#define CORE_VARIABLES_MAX 128
#define CORE_AUDIO_BATCH   128

struct Core {
	char *game_data;
	size_t game_data_size;

	struct retro_system_info system_info;

	void (RETRO_CALLCONV *retro_set_environment)(retro_environment_t);
	void (RETRO_CALLCONV *retro_set_video_refresh)(retro_video_refresh_t);
	void (RETRO_CALLCONV *retro_set_audio_sample)(retro_audio_sample_t);
	void (RETRO_CALLCONV *retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
	void (RETRO_CALLCONV *retro_set_input_poll)(retro_input_poll_t);
	void (RETRO_CALLCONV *retro_set_input_state)(retro_input_state_t);
	void (RETRO_CALLCONV *retro_init)(void);
	void (RETRO_CALLCONV *retro_deinit)(void);
	unsigned (RETRO_CALLCONV *retro_api_version)(void);
	void (RETRO_CALLCONV *retro_get_system_info)(struct retro_system_info *info);
	void (RETRO_CALLCONV *retro_get_system_av_info)(struct retro_system_av_info *info);
	void (RETRO_CALLCONV *retro_set_controller_port_device)(unsigned port, unsigned device);
	void (RETRO_CALLCONV *retro_reset)(void);
	void (RETRO_CALLCONV *retro_run)(void);
	size_t (RETRO_CALLCONV *retro_serialize_size)(void);
	bool (RETRO_CALLCONV *retro_serialize)(void *data, size_t size);
	bool (RETRO_CALLCONV *retro_unserialize)(const void *data, size_t size);
	//void (RETRO_CALLCONV *retro_cheat_reset)(void);
	//void (RETRO_CALLCONV *retro_cheat_set)(unsigned index, bool enabled, const char *code);
	bool (RETRO_CALLCONV *retro_load_game)(const struct retro_game_info *game);
	//bool (RETRO_CALLCONV *retro_load_game_special)(unsigned game_type,
	//	const struct retro_game_info *info, size_t num_info);
	void (RETRO_CALLCONV *retro_unload_game)(void);
	unsigned (RETRO_CALLCONV *retro_get_region)(void);
	void *(RETRO_CALLCONV *retro_get_memory_data)(unsigned id);
	size_t (RETRO_CALLCONV *retro_get_memory_size)(unsigned id);
};


// Globals from the environment callback

static enum retro_pixel_format RETRO_PIXEL_FORMAT = RETRO_PIXEL_FORMAT_0RGB1555;
static struct retro_game_geometry RETRO_GAME_GEOMETRY;
static struct retro_system_timing RETRO_SYSTEM_TIMING;
static struct retro_disk_control_callback RETRO_DISK_CONTROL_CALLBACK;
static unsigned RETRO_CONTROLLER_DEVICE = RETRO_DEVICE_JOYPAD;
static unsigned RETRO_REGION;

static uint32_t CORE_NUM_VARIABLES;
static CoreSetting CORE_VARIABLES[CORE_VARIABLES_MAX];
static bool CORE_OPT_SET;

static CoreLogFunc CORE_LOG;
static CoreAudioFunc CORE_AUDIO;
static CoreVideoFunc CORE_VIDEO;
static void *CORE_LOG_OPAQUE;
static void *CORE_AUDIO_OPAQUE;
static void *CORE_VIDEO_OPAQUE;

static char CORE_SAVE_DIR[MTY_PATH_MAX];
static char CORE_SYSTEM_DIR[MTY_PATH_MAX];

static bool CORE_BUTTONS[2][CORE_PLAYERS_MAX][CORE_BUTTON_MAX];
static int16_t CORE_AXES[CORE_PLAYERS_MAX][CORE_AXIS_MAX];

static size_t CORE_NUM_FRAMES;
static int16_t CORE_FRAMES[CORE_SAMPLES_MAX];


// Maps

static const CoreButton CORE_BUTTON_MAP[16] = {
	[RETRO_DEVICE_ID_JOYPAD_B]      = CORE_BUTTON_B,
	[RETRO_DEVICE_ID_JOYPAD_Y]      = CORE_BUTTON_Y,
	[RETRO_DEVICE_ID_JOYPAD_SELECT] = CORE_BUTTON_SELECT,
	[RETRO_DEVICE_ID_JOYPAD_START]  = CORE_BUTTON_START,
	[RETRO_DEVICE_ID_JOYPAD_UP]     = CORE_BUTTON_DPAD_U,
	[RETRO_DEVICE_ID_JOYPAD_DOWN]   = CORE_BUTTON_DPAD_D,
	[RETRO_DEVICE_ID_JOYPAD_LEFT]   = CORE_BUTTON_DPAD_L,
	[RETRO_DEVICE_ID_JOYPAD_RIGHT]  = CORE_BUTTON_DPAD_R,
	[RETRO_DEVICE_ID_JOYPAD_A]      = CORE_BUTTON_A,
	[RETRO_DEVICE_ID_JOYPAD_X]      = CORE_BUTTON_X,
	[RETRO_DEVICE_ID_JOYPAD_L]      = CORE_BUTTON_L,
	[RETRO_DEVICE_ID_JOYPAD_R]      = CORE_BUTTON_R,
	[RETRO_DEVICE_ID_JOYPAD_L2]     = CORE_BUTTON_L2,
	[RETRO_DEVICE_ID_JOYPAD_R2]     = CORE_BUTTON_R2,
	[RETRO_DEVICE_ID_JOYPAD_L3]     = CORE_BUTTON_L3,
	[RETRO_DEVICE_ID_JOYPAD_R3]     = CORE_BUTTON_R3,
};

static const CoreAxis CORE_AXIS_MAP[3][2] = {
	[RETRO_DEVICE_INDEX_ANALOG_LEFT] = {
		[RETRO_DEVICE_ID_ANALOG_X] = CORE_AXIS_LX,
		[RETRO_DEVICE_ID_ANALOG_Y] = CORE_AXIS_LY,
	},
	[RETRO_DEVICE_INDEX_ANALOG_RIGHT] = {
		[RETRO_DEVICE_ID_ANALOG_X] = CORE_AXIS_RX,
		[RETRO_DEVICE_ID_ANALOG_Y] = CORE_AXIS_RY,
	},
	[RETRO_DEVICE_INDEX_ANALOG_BUTTON] = {
		[RETRO_DEVICE_ID_ANALOG_X] = 0,
		[RETRO_DEVICE_ID_ANALOG_Y] = 0,
	},
};


// libretro callbacks

static void rcore_retro_log_printf(enum retro_log_level level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char *msg = MTY_VsprintfD(fmt, args);

	va_end(args);

	if (level != RETRO_LOG_DEBUG)
		CORE_LOG(msg, CORE_LOG_OPAQUE);

	MTY_Free(msg);
}

static bool rcore_retro_rumble(unsigned port, enum retro_rumble_effect effect, uint16_t strength)
{
	// TODO

	return false;
}

static uintptr_t rcore_retro_hw_get_current_framebuffer(void)
{
	// TODO

	return 0;
}

static retro_proc_address_t rcore_retro_hw_get_proc_address(const char *sym)
{
	// TODO

	return NULL;
}

static const char *rcore_setting_default(const char *key)
{
	// Hardware rendering is not set up, make sure to use
	// available software renderers

	if (!strcmp(key, "mupen64plus-rdp-plugin"))
		return "angrylion";

	if (!strcmp(key, "mupen64plus-rsp-plugin"))
		return "cxd4";

	return NULL;
}

static void rcore_parse_setting(CoreSetting *var, const char *key, const char *val)
{
	snprintf(var->key, CORE_KEY_NAME_MAX, "%s", key);
	const char *def = rcore_setting_default(key);

	char *dup = MTY_Strdup(val);

	char *semi = strstr(dup, ";");
	if (semi) {
		*semi = '\0';
		snprintf(var->desc, CORE_DESC_MAX, "%s", dup);

		semi++;
		while (*semi == ' ')
			semi++;

		char *ptr = NULL;
		char *tok = MTY_Strtok(semi, "|", &ptr);

		while (tok && var->nopts < CORE_OPTS_MAX) {
			if (var->nopts == 0)
				snprintf(var->value, CORE_KEY_NAME_MAX, "%s", def ? def : tok);

			snprintf(var->opts[var->nopts++], CORE_OPT_NAME_MAX, "%s", tok);
			tok = MTY_Strtok(NULL, "|", &ptr);
		}
	}

	MTY_Free(dup);
}

static bool rcore_retro_environment(unsigned cmd, void *data)
{
	switch (cmd) {
		case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
		case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
		case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: {
			unsigned *arg = data;
			*arg = 0;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
			struct retro_log_callback *arg = data;
			arg->log = rcore_retro_log_printf;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_MESSAGE: {
			const struct retro_message *arg = data;

			CORE_LOG(arg->msg, CORE_LOG_OPAQUE);

			return true;
		}
		case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
			bool *arg = data;
			*arg = true;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
		case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
			const char **arg = data;

			*arg = CORE_SYSTEM_DIR;
			MTY_Mkdir(*arg);

			return true;
		}
		case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
			const char **arg = data;

			*arg = CORE_SAVE_DIR;
			MTY_Mkdir(*arg);

			return true;
		}
		case RETRO_ENVIRONMENT_GET_VARIABLE: {
			struct retro_variable *arg = data;
			arg->value = NULL;

			for (uint32_t x = 0; x < CORE_NUM_VARIABLES; x++) {
				CoreSetting *s = &CORE_VARIABLES[x];

				if (!strcmp(arg->key, s->key)) {
					arg->value = s->value;
					break;
				}
			}

			return arg->value ? true : false;
		}
		case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
			int *arg = data;
			*arg = 0x3;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_LANGUAGE: {
			unsigned *arg = data;
			*arg = RETRO_LANGUAGE_ENGLISH;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_VARIABLES: {
			const struct retro_variable *arg = data;

			for (uint32_t x = 0; x < UINT32_MAX; x++) {
				const struct retro_variable *v = &arg[x];
				if (!v->key || !v->value)
					break;

				if (CORE_NUM_VARIABLES < CORE_VARIABLES_MAX) {
					rcore_parse_setting(&CORE_VARIABLES[CORE_NUM_VARIABLES], v->key, v->value);
					CORE_NUM_VARIABLES++;
				}
			}

			return true;
		}
		case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
			bool *arg = data;
			*arg = CORE_OPT_SET;
			CORE_OPT_SET = false;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
			const enum retro_pixel_format *arg = data;

			RETRO_PIXEL_FORMAT = *arg;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_GEOMETRY: {
			const struct retro_game_geometry *arg = data;
			RETRO_GAME_GEOMETRY = *arg;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
			const struct retro_system_av_info *arg = data;
			RETRO_GAME_GEOMETRY = arg->geometry;
			RETRO_SYSTEM_TIMING = arg->timing;

			return true;
		}
		case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
			const struct retro_disk_control_callback *arg = data;
			RETRO_DISK_CONTROL_CALLBACK = *arg;

			return true;
		}
		case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
			struct retro_rumble_interface *arg = data;
			arg->set_rumble_state = rcore_retro_rumble;

			return true;
		}

		// TODO
		case RETRO_ENVIRONMENT_SET_HW_RENDER: {
			printf("RETRO_ENVIRONMENT_SET_HW_RENDER\n");

			struct retro_hw_render_callback *arg = data;

			arg->get_current_framebuffer = rcore_retro_hw_get_current_framebuffer;
			arg->get_proc_address = rcore_retro_hw_get_proc_address;

			// arg->context_reset();

			break;
		}
		case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE: {
			printf("RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE\n");
			const struct retro_hw_render_context_negotiation_interface *arg = data;

			printf("\t%d, 0x%X\n", arg->interface_type, arg->interface_version);

			break;
		}
		case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: {
			printf("RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO\n");
			const struct retro_subsystem_info *arg = data;
			// printf("\t%s, %s, %u\n", arg->desc, arg->ident, arg->id);

			for (unsigned x = 0; x < arg->num_roms; x++) {
				// const struct retro_subsystem_rom_info *rom = &arg->roms[x];
				// printf("\t\t%s\n", rom->desc);
			}
			break;
		}
		case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {
			printf("RETRO_ENVIRONMENT_SET_CONTROLLER_INFO\n");
			const struct retro_controller_info *arg = data;

			for (unsigned x = 0; x < arg->num_types; x++) {
				const struct retro_controller_description *type = &arg->types[x];

				if (type->id == RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 0))
					RETRO_CONTROLLER_DEVICE = type->id;

				// printf("\t%s: %u\n", type->desc, type->id);
			}
			return true;
		}
		case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
			printf("RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS\n");
			const struct retro_input_descriptor *arg = data;

			for (size_t x = 0; true; x++) {
				const struct retro_input_descriptor *desc = &arg[x];
				if (!desc->description)
					break;

				// printf("\t%u %u %u %u %s\n", desc->port, desc->device,
				// 	desc->index, desc->id, desc->description);
			}
			break;
		}
		case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
			printf("RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE\n");
			break;
		case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
			printf("RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER\n");
			break;
		case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
			printf("RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS\n");
			break;
		// case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
		// 	printf("RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE\n");
		// 	break;
		// case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
		// 	printf("RETRO_ENVIRONMENT_GET_GAME_INFO_EXT\n");
		// 	break;
		case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
			printf("RETRO_ENVIRONMENT_GET_LED_INTERFACE\n");
			break;
		case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
			printf("RETRO_ENVIRONMENT_GET_PERF_INTERFACE\n");
			break;

		// Unimplemented
		case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
			// Performance demands hint
		case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
			// This is an optional efficiency for polling controllers
		// case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
		case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
			// Optionally hide certain settings, dynamically restore them
		case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
			// Used to implement a "Virtual File System" with custom IO functions
		case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
			// Achievement system support
		case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
			// NES / SNES memory maps for cheats and other advanced emulator features
		case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
		case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
			// Handled by resampling and libmatoya's min/max buffers
		case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
			// Preallocated memory for drawing, overhead is minimal on current systems
		case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
			// Fastforwarding mode
			break;

		// Unknown
		default:
			printf("RETRO_ENVIRONMENT_UNKNOWN: (0x%X) (%u)\n", cmd, cmd & 0xFF);
			break;
	}

	return false;
}

static CoreColorFormat rcore_color_format(void)
{
	switch (RETRO_PIXEL_FORMAT) {
		case RETRO_PIXEL_FORMAT_XRGB8888: return CORE_COLOR_FORMAT_BGRA;
		case RETRO_PIXEL_FORMAT_RGB565:   return CORE_COLOR_FORMAT_B5G6R5;
		case RETRO_PIXEL_FORMAT_0RGB1555: return CORE_COLOR_FORMAT_B5G5R5A1;
		default:
			break;
	}

	return CORE_COLOR_FORMAT_UNKNOWN;
}

static void rcore_retro_video_refresh(const void *data, unsigned width,
	unsigned height, size_t pitch)
{
	CORE_VIDEO(data, rcore_color_format(), width, height, pitch, CORE_VIDEO_OPAQUE);
}

static void rcore_output_audio(size_t batch)
{
	if (CORE_NUM_FRAMES > batch) {
		CORE_AUDIO(CORE_FRAMES, CORE_NUM_FRAMES, lrint(RETRO_SYSTEM_TIMING.sample_rate), CORE_AUDIO_OPAQUE);
		CORE_NUM_FRAMES = 0;
	}
}

static void rcore_retro_audio_sample(int16_t left, int16_t right)
{
	if (CORE_NUM_FRAMES + 1 <= CORE_FRAMES_MAX) {
		CORE_FRAMES[CORE_NUM_FRAMES * 2] = left;
		CORE_FRAMES[CORE_NUM_FRAMES * 2 + 1] = right;
		CORE_NUM_FRAMES++;
	}

	rcore_output_audio(CORE_AUDIO_BATCH);
}

static size_t rcore_retro_audio_sample_batch(const int16_t *data, size_t frames)
{
	if (CORE_NUM_FRAMES + frames <= CORE_FRAMES_MAX) {
		memcpy(CORE_FRAMES + CORE_NUM_FRAMES * 2, data, frames * 4);
		CORE_NUM_FRAMES += frames;
	}

	rcore_output_audio(CORE_AUDIO_BATCH);

	return frames;
}

static void rcore_retro_input_poll(void)
{
}

static int16_t rcore_retro_input_state(unsigned port, unsigned device,
	unsigned index, unsigned id)
{
	if (port >= CORE_PLAYERS_MAX)
		return 0;

	// Buttons
	if (device == RETRO_DEVICE_JOYPAD) {
		if (id >= 16)
			return 0;

		return CORE_BUTTONS[0][port][CORE_BUTTON_MAP[id]];

	// Axes
	} else if (device == RETRO_DEVICE_ANALOG) {
		if (index >= 3 || id >= 2)
			return 0;

		return CORE_AXES[port][CORE_AXIS_MAP[index][id]];
	}

	return 0;
}


// Core API

static MTY_SO *SO;

void rcore_set_so(MTY_SO *so)
{
	SO = so;
}

static bool rcore_load_symbols(MTY_SO *so, Core *ctx)
{
	#define CORE_LOAD_SYM(sym) \
		ctx->sym = MTY_SOGetSymbol(so, #sym); \
		if (!ctx->sym) return false

	CORE_LOAD_SYM(retro_set_environment);
	CORE_LOAD_SYM(retro_set_video_refresh);
	CORE_LOAD_SYM(retro_set_audio_sample);
	CORE_LOAD_SYM(retro_set_audio_sample_batch);
	CORE_LOAD_SYM(retro_set_input_poll);
	CORE_LOAD_SYM(retro_set_input_state);
	CORE_LOAD_SYM(retro_init);
	CORE_LOAD_SYM(retro_deinit);
	CORE_LOAD_SYM(retro_api_version);
	CORE_LOAD_SYM(retro_get_system_info);
	CORE_LOAD_SYM(retro_get_system_av_info);
	CORE_LOAD_SYM(retro_set_controller_port_device);
	CORE_LOAD_SYM(retro_reset);
	CORE_LOAD_SYM(retro_run);
	CORE_LOAD_SYM(retro_serialize_size);
	CORE_LOAD_SYM(retro_serialize);
	CORE_LOAD_SYM(retro_unserialize);
	CORE_LOAD_SYM(retro_load_game);
	CORE_LOAD_SYM(retro_unload_game);
	CORE_LOAD_SYM(retro_get_region);
	CORE_LOAD_SYM(retro_get_memory_data);
	CORE_LOAD_SYM(retro_get_memory_size);

	return true;
}

void rcore_unload_game(Core **core)
{
	if (!core || !*core)
		return;

	Core *ctx = *core;

	if (ctx->retro_unload_game)
		ctx->retro_unload_game();

	if (ctx->retro_deinit)
		ctx->retro_deinit();

	MTY_Free(ctx->game_data);

	// Globals
	RETRO_PIXEL_FORMAT = RETRO_PIXEL_FORMAT_0RGB1555;
	memset(&RETRO_GAME_GEOMETRY, 0, sizeof(struct retro_game_geometry));
	memset(&RETRO_SYSTEM_TIMING, 0, sizeof(struct retro_system_timing));
	memset(&RETRO_DISK_CONTROL_CALLBACK, 0, sizeof(struct retro_disk_control_callback));
	RETRO_CONTROLLER_DEVICE = RETRO_DEVICE_JOYPAD;
	RETRO_REGION = 0;

	CORE_NUM_VARIABLES = 0;
	memset(CORE_VARIABLES, 0, sizeof(CoreSetting) * CORE_VARIABLES_MAX);

	CORE_OPT_SET = false;

	CORE_LOG = NULL;
	CORE_AUDIO = NULL;
	CORE_VIDEO = NULL;
	CORE_LOG_OPAQUE = NULL;
	CORE_AUDIO_OPAQUE = NULL;
	CORE_VIDEO_OPAQUE = NULL;
	CORE_NUM_FRAMES = 0;

	memset(CORE_BUTTONS, 0, sizeof(bool) * 2 * CORE_PLAYERS_MAX * CORE_BUTTON_MAX);
	memset(CORE_AXES, 0, sizeof(int16_t) * CORE_PLAYERS_MAX * CORE_AXIS_MAX);

	MTY_Free(ctx);
	*core = NULL;
}

Core *rcore_load_game(CoreSystem system, const char *system_dir, const char *path,
	const void *save_data, size_t save_data_size)
{
	Core *ctx = MTY_Alloc(1, sizeof(Core));

	bool r = true;

	snprintf(CORE_SAVE_DIR, MTY_PATH_MAX, "%s", system_dir);
	snprintf(CORE_SYSTEM_DIR, MTY_PATH_MAX, "%s", system_dir);

	r = rcore_load_symbols(SO, ctx);
	if (!r)
		goto except;

	r = ctx->retro_api_version() == RETRO_API_VERSION;
	if (!r)
		goto except;

	ctx->retro_set_environment(rcore_retro_environment);
	ctx->retro_init();

	ctx->retro_set_video_refresh(rcore_retro_video_refresh);
	ctx->retro_set_audio_sample(rcore_retro_audio_sample);
	ctx->retro_set_audio_sample_batch(rcore_retro_audio_sample_batch);
	ctx->retro_set_input_poll(rcore_retro_input_poll);
	ctx->retro_set_input_state(rcore_retro_input_state);

	ctx->retro_get_system_info(&ctx->system_info);

	MTY_Free(ctx->game_data);
	ctx->game_data = NULL;
	ctx->game_data_size = 0;

	struct retro_game_info game = {0};
	game.path = path;
	game.meta = "merton";

	if (!ctx->system_info.need_fullpath) {
		ctx->game_data = MTY_ReadFile(path, &ctx->game_data_size);
		if (!ctx->game_data) {
			r = false;
			goto except;
		}

		game.data = ctx->game_data;
		game.size = ctx->game_data_size;
	}

	r = ctx->retro_load_game(&game);
	if (!r)
		goto except;

	ctx->retro_set_controller_port_device(0, RETRO_CONTROLLER_DEVICE);

	struct retro_system_av_info av_info = {0};
	ctx->retro_get_system_av_info(&av_info);
	RETRO_SYSTEM_TIMING = av_info.timing;
	RETRO_GAME_GEOMETRY = av_info.geometry;

	RETRO_REGION = ctx->retro_get_region();

	// Load SRAM
	size_t sram_size = ctx->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

	if (sram_size > 0 && sram_size >= save_data_size) {
		void *sram = ctx->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);

		if (sram)
			memcpy(sram, save_data, save_data_size);
	}

	except:

	if (!r)
		rcore_unload_game(&ctx);

	return ctx;
}

void rcore_reset(Core *ctx)
{
	if (!ctx)
		return;

	ctx->retro_reset();
}

void rcore_run(Core *ctx)
{
	if (!ctx)
		return;

	ctx->retro_run();
	rcore_output_audio(0);

	memcpy(CORE_BUTTONS[0], CORE_BUTTONS[1], sizeof(bool) * CORE_PLAYERS_MAX * CORE_BUTTON_MAX);
}

double rcore_get_frame_rate(Core *ctx)
{
	if (!ctx)
		return 60.0;

	return RETRO_SYSTEM_TIMING.fps;
}

float rcore_get_aspect_ratio(Core *ctx)
{
	if (!ctx)
		return 1.0f;

	float ar = RETRO_GAME_GEOMETRY.aspect_ratio;

	if (ar <= 0.0f)
		ar = (float) RETRO_GAME_GEOMETRY.base_width / (float) RETRO_GAME_GEOMETRY.base_height;

	return ar;
}

void rcore_set_button(Core *ctx, uint8_t player, CoreButton button, bool pressed)
{
	if (!ctx)
		return;

	CORE_BUTTONS[0][player][button] |= pressed;
	CORE_BUTTONS[1][player][button] = pressed;
}

void rcore_set_axis(Core *ctx, uint8_t player, CoreAxis axis, int16_t value)
{
	if (!ctx)
		return;

	CORE_AXES[player][axis] = value;
}

void *rcore_get_state(Core *ctx, size_t *size)
{
	if (!ctx)
		return NULL;

	*size = ctx->retro_serialize_size();
	if (*size == 0)
		return NULL;

	void *state = MTY_Alloc(*size, 1);
	if (!ctx->retro_serialize(state, *size)) {
		MTY_Free(state);
		return NULL;
	}

	return state;
}

bool rcore_set_state(Core *ctx, const void *state, size_t size)
{
	if (!ctx)
		return false;

	return ctx->retro_unserialize(state, size);
}

bool rcore_insert_disc(Core *ctx, const char *path)
{
	if (!ctx)
		return false;

	if (RETRO_DISK_CONTROL_CALLBACK.set_eject_state)
		RETRO_DISK_CONTROL_CALLBACK.set_eject_state(true);

	// TODO This is where we would load PSX disks

	return false;
}

void *rcore_get_save_data(Core *ctx, size_t *size)
{
	if (!ctx)
		return NULL;

	*size = ctx->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
	if (*size == 0)
		return NULL;

	const void *sdata = ctx->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
	if (!sdata)
		return NULL;

	void *copy = MTY_Alloc(*size, 1);
	memcpy(copy, sdata, *size);

	return copy;
}

void rcore_set_log_func(CoreLogFunc func, void *opaque)
{
	CORE_LOG = func;
	CORE_LOG_OPAQUE = opaque;
}

void rcore_set_audio_func(CoreAudioFunc func, void *opaque)
{
	CORE_AUDIO = func;
	CORE_AUDIO_OPAQUE = opaque;
}

void rcore_set_video_func(CoreVideoFunc func, void *opaque)
{
	CORE_VIDEO = func;
	CORE_VIDEO_OPAQUE = opaque;
}

CoreSetting *rcore_get_settings(uint32_t *len)
{
	*len = CORE_NUM_VARIABLES;

	return CORE_VARIABLES;
}

void rcore_update_settings(Core *ctx)
{
	CORE_OPT_SET = true;
}
