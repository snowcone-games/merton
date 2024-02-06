#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(CORE_EXPORT)
	#if defined(_MSC_VER)
		#define EXPORT __declspec(dllexport)
	#else
		#define EXPORT __attribute__((visibility("default")))
	#endif
#elif defined(CORE_EXPORT_EXTERN)
	#define EXPORT extern
#else
	#define EXPORT
#endif

#if defined(CORE_FP)
	#define FP(func) (*func)
#else
	#define FP(func) func
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CORE_PLAYERS_MAX   8
#define CORE_DESC_MAX      128
#define CORE_OPTS_MAX      32
#define CORE_KEY_NAME_MAX  64
#define CORE_OPT_NAME_MAX  64
#define CORE_FRAMES_MAX    0x3000
#define CORE_SAMPLES_MAX   (CORE_FRAMES_MAX * 2)

typedef struct Core Core;

typedef enum {
	CORE_BUTTON_A      = 1,
	CORE_BUTTON_B      = 2,
	CORE_BUTTON_X      = 3,
	CORE_BUTTON_Y      = 4,
	CORE_BUTTON_L      = 5,
	CORE_BUTTON_R      = 6,
	CORE_BUTTON_L2     = 7,
	CORE_BUTTON_R2     = 8,
	CORE_BUTTON_L3     = 9,
	CORE_BUTTON_R3     = 10,
	CORE_BUTTON_DPAD_U = 11,
	CORE_BUTTON_DPAD_D = 12,
	CORE_BUTTON_DPAD_L = 13,
	CORE_BUTTON_DPAD_R = 14,
	CORE_BUTTON_SELECT = 15,
	CORE_BUTTON_START  = 16,
	CORE_BUTTON_MAX,
} CoreButton;

typedef enum {
	CORE_AXIS_LX = 1,
	CORE_AXIS_LY = 2,
	CORE_AXIS_RX = 3,
	CORE_AXIS_RY = 4,
	CORE_AXIS_MAX,
} CoreAxis;

typedef enum {
	CORE_COLOR_FORMAT_UNKNOWN  = 0,
	CORE_COLOR_FORMAT_BGRA     = 1, // 32-bit, 8-bits per channel
	CORE_COLOR_FORMAT_RGBA     = 2, // 32-bit, 8-bits per channel
	CORE_COLOR_FORMAT_B5G6R5   = 3, // 16-bit
	CORE_COLOR_FORMAT_B5G5R5A1 = 4, // 16-bit
} CoreColorFormat;

typedef enum {
	CORE_SYSTEM_UNKNOWN   = 0,
	CORE_SYSTEM_ATARI2600 = 1,
	CORE_SYSTEM_NES       = 2,  // Nintendo Entertainment System
	CORE_SYSTEM_SMS       = 3,  // Sega Master System
	CORE_SYSTEM_TG16      = 4,  // TurboGrafx-16 (PC Engine)
	CORE_SYSTEM_GENESIS   = 5,
	CORE_SYSTEM_GAMEBOY   = 6,
	CORE_SYSTEM_SNES      = 7,  // Super Nintendo Entertainment System
	CORE_SYSTEM_PS        = 8,  // PlayStation
	CORE_SYSTEM_N64       = 9,  // Nintendo 64
	CORE_SYSTEM_GBA       = 10,
	CORE_SYSTEM_MAX,
} CoreSystem;

typedef enum {
	CORE_SETTING_ENUM  = 0,
	CORE_SETTING_BOOL  = 1,
} CoreSettingType;

typedef enum {
	CORE_SETTING_GROUP_GENERAL  = 0,
	CORE_SETTING_GROUP_ADVANCED = 1,
	CORE_SETTING_GROUP_VIDEO    = 2,
	CORE_SETTING_GROUP_AUDIO    = 3,
	CORE_SETTING_GROUP_INPUT    = 4,
} CoreSettingGroup;

typedef struct {
	CoreSystem system;
	CoreSettingType type;
	CoreSettingGroup group;
	char desc[CORE_DESC_MAX];
	char key[CORE_KEY_NAME_MAX];
	char opts[CORE_OPTS_MAX][CORE_OPT_NAME_MAX];
	char value[CORE_OPT_NAME_MAX];
	uint32_t nopts;
} CoreSetting;


// Callback with log messages
typedef void (*CoreLogFunc)(const char *msg, void *opaque);

// Callback with 16-bit signed PCM audio samples
typedef void (*CoreAudioFunc)(const int16_t *buf, size_t frames, uint32_t sampleRate,
	void *opaque);

// Callback with a video frame
typedef void (*CoreVideoFunc)(const void *buf, CoreColorFormat format, uint32_t width,
	uint32_t height, size_t pitch, void *opaque);


// Core initialization
// 'systemDir' is where to look for BIOS files

EXPORT Core *
FP(CoreLoad)(const char *systemDir);

EXPORT void
FP(CoreUnload)(Core **core);


// Output callbacks

EXPORT void
FP(CoreSetLogFunc)(Core *ctx, CoreLogFunc func, void *opaque);

EXPORT void
FP(CoreSetAudioFunc)(Core *ctx, CoreAudioFunc func, void *opaque);

EXPORT void
FP(CoreSetVideoFunc)(Core *ctx, CoreVideoFunc func, void *opaque);


// Game loading
// 'saveData' is written upfront rather than written to after the game is loaded

EXPORT bool
FP(CoreLoadGame)(Core *ctx, CoreSystem system, const char *path, const void *saveData,
	size_t saveDataSize);

EXPORT void
FP(CoreUnloadGame)(Core *ctx);


// Video information for frontend presentation

EXPORT double
FP(CoreGetFrameRate)(Core *ctx);

EXPORT float
FP(CoreGetAspectRatio)(Core *ctx);


// Run a single frame

EXPORT void
FP(CoreRun)(Core *ctx);


// Get save data to persist on disk

EXPORT void *
FP(CoreGetSaveData)(Core *ctx, size_t *size);


// Equivalent to pressing the reset button on the console

EXPORT void
FP(CoreReset)(Core *ctx);


// Input

EXPORT void
FP(CoreSetButton)(Core *ctx, uint8_t player, CoreButton button, bool pressed);

EXPORT void
FP(CoreSetAxis)(Core *ctx, uint8_t player, CoreAxis axis, int16_t value);


// Save states

EXPORT void *
FP(CoreGetState)(Core *ctx, size_t *size);

EXPORT bool
FP(CoreSetState)(Core *ctx, const void *state, size_t size);


// CD interface

EXPORT bool
FP(CoreInsertDisc)(Core *ctx, const char *path);


// Settings

EXPORT CoreSetting *
FP(CoreGetSettings)(uint32_t *len);

EXPORT void
FP(CoreUpdateSettings)(Core *ctx);

#ifdef __cplusplus
}
#endif
