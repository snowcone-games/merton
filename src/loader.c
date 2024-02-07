#include "matoya.h"

#define CORE_FP
#include "core.h"
#include "rcore.h"

#include "loader.h"

static MTY_SO *LOADER_SO;

static void loader_set_rcore(void)
{
	#define LOADER_SET_RCORE(sym, rsym) \
		sym = rsym

	LOADER_SET_RCORE(CoreLoadGame, rcore_load_game);
	LOADER_SET_RCORE(CoreUnloadGame, rcore_unload_game);
	LOADER_SET_RCORE(CoreReset, rcore_reset);
	LOADER_SET_RCORE(CoreRun, rcore_run);
	LOADER_SET_RCORE(CoreSetButton, rcore_set_button);
	LOADER_SET_RCORE(CoreSetAxis, rcore_set_axis);
	LOADER_SET_RCORE(CoreGetState, rcore_get_state);
	LOADER_SET_RCORE(CoreSetState, rcore_set_state);
	LOADER_SET_RCORE(CoreInsertDisc, rcore_insert_disc);
	LOADER_SET_RCORE(CoreGetSaveData, rcore_get_save_data);
	LOADER_SET_RCORE(CoreGetFrameRate, rcore_get_frame_rate);
	LOADER_SET_RCORE(CoreGetAspectRatio, rcore_get_aspect_ratio);
	LOADER_SET_RCORE(CoreSetLogFunc, rcore_set_log_func);
	LOADER_SET_RCORE(CoreSetAudioFunc, rcore_set_audio_func);
	LOADER_SET_RCORE(CoreSetVideoFunc, rcore_set_video_func);
	LOADER_SET_RCORE(CoreGetSettings, rcore_get_settings);
	LOADER_SET_RCORE(CoreUpdateSettings, rcore_update_settings);
}

bool loader_load(const char *name)
{
	loader_reset();

	if (!name)
		return false;

	LOADER_SO = MTY_SOLoad(name);
	if (!LOADER_SO)
		return false;

	MTY_DisableLog(true);
	void *sym = (void *) MTY_SOGetSymbol(LOADER_SO, "retro_api_version");
	MTY_DisableLog(false);

	if (sym) {
		rcore_set_so(LOADER_SO);
		return true;
	}

	#define LOADER_LOAD_SYM(sym) \
		sym = MTY_SOGetSymbol(LOADER_SO, #sym); \
		if (!sym) return false

	LOADER_LOAD_SYM(CoreLoadGame);
	LOADER_LOAD_SYM(CoreUnloadGame);
	LOADER_LOAD_SYM(CoreReset);
	LOADER_LOAD_SYM(CoreRun);
	LOADER_LOAD_SYM(CoreSetButton);
	LOADER_LOAD_SYM(CoreSetAxis);
	LOADER_LOAD_SYM(CoreGetState);
	LOADER_LOAD_SYM(CoreSetState);
	LOADER_LOAD_SYM(CoreInsertDisc);
	LOADER_LOAD_SYM(CoreGetSaveData);
	LOADER_LOAD_SYM(CoreGetFrameRate);
	LOADER_LOAD_SYM(CoreGetAspectRatio);
	LOADER_LOAD_SYM(CoreSetLogFunc);
	LOADER_LOAD_SYM(CoreSetAudioFunc);
	LOADER_LOAD_SYM(CoreSetVideoFunc);
	LOADER_LOAD_SYM(CoreGetSettings);
	LOADER_LOAD_SYM(CoreUpdateSettings);

	return true;
}

void loader_reset(void)
{
	loader_set_rcore();
	MTY_SOUnload(&LOADER_SO);
}
