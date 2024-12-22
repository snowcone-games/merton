#include "csync.h"

#include <stdio.h>
#include <string.h>

#include "matoya.h"
#include "config.h"

#if defined(__arm64__)
	#define CSYNC_CORE_ARCH "arm64"
#else
	#define CSYNC_CORE_ARCH "x86_64"
#endif

#if defined(_WIN32)
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

enum csync_cmd_type {
	CSYNC_CMD_STOP,
	CSYNC_CMD_FETCH_CORE,
	CSYNC_CMD_FETCH_CORE_HASH,
};

struct csync {
	MTY_Thread *thread;
	MTY_Queue *q;
	MTY_Mutex *m;

	MTY_JSON *core_hash;

	MTY_Atomic32 fetch_core;
};

struct csync_cmd {
	enum csync_cmd_type type;

	union {
		struct {
			char file[MTY_PATH_MAX];
		} fetch_core;
	};
};


// HTTP helpers

static bool csync_fetch(const char *url, void **response, size_t *size)
{
	MTY_Log("Fetching '%s'", url);

	uint16_t status = 0;
	bool ok = MTY_HttpRequest(url, "GET", NULL, NULL, 0, NULL, 10000, response, size, &status);

	return ok && status == 200;
}


// Fetch core

static const char *csync_get_platform(void)
{
	uint32_t platform = MTY_GetPlatform();

	switch (platform & 0xFF000000) {
		case MTY_OS_WINDOWS: return "windows";
		case MTY_OS_MACOS:   return "macosx";
		case MTY_OS_ANDROID: return "android";
		case MTY_OS_LINUX:   return "linux";
		case MTY_OS_WEB:     return "web";
		case MTY_OS_IOS:     return "ios";
		case MTY_OS_TVOS:    return "tvos";
	}

	return "unknown";
}

static void csync_fetch_core_cmd(struct csync *ctx, struct csync_cmd *cmd)
{
	const char *url = MTY_SprintfDL("https://snowcone.ltd/cores/%s/%s/%s",
		csync_get_platform(), CSYNC_CORE_ARCH, cmd->fetch_core.file);

	void *so = NULL;
	size_t size = 0;

	if (csync_fetch(url, &so, &size)) {
		const char *base = config_cores_dir();
		MTY_Mkdir(base);

		const char *path = MTY_JoinPath(base, cmd->fetch_core.file);

		if (MTY_WriteFile(path, so, size))
			MTY_Atomic32Set(&ctx->fetch_core, 1);
	}
}

void csync_fetch_core(struct csync *ctx, const char *file)
{
	struct csync_cmd *cmd = MTY_QueueGetInputBuffer(ctx->q, sizeof(struct csync_cmd));

	if (cmd) {
		cmd->type = CSYNC_CMD_FETCH_CORE;
		snprintf(cmd->fetch_core.file, MTY_PATH_MAX, "%s", file);

		MTY_QueuePush(ctx->q);
	}
}

bool csync_poll_fetch_core(struct csync *ctx)
{
	if (MTY_Atomic32Get(&ctx->fetch_core) == 1) {
		MTY_Atomic32Set(&ctx->fetch_core, 0);

		return true;
	}

	return false;
}


// Core hash

static void csync_fetch_core_hash_cmd(struct csync *ctx, struct csync_cmd *cmd)
{
	const char *url = MTY_SprintfDL("https://snowcone.ltd/cores/core-hash.json");

	void *json = NULL;
	size_t size = 0;

	if (csync_fetch(url, &json, &size)) {
		MTY_MutexLock(ctx->m);

		ctx->core_hash = MTY_JSONParse(json);

		MTY_MutexUnlock(ctx->m);
	}
}

static bool csync_is_symlink(const char *file)
{
	#if defined(_WIN32)
		DWORD attr = GetFileAttributes(MTY_MultiToWideDL(file));

		return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_REPARSE_POINT);
	#else
		struct stat s;
		return lstat(file, &s) == 0 && S_ISLNK(s.st_mode);
	#endif
}

bool csync_check_core_hash(struct csync *ctx, const char *core, const char *file)
{
	if (csync_is_symlink(file)) {
		MTY_Log("'%s' is a symlink, hash not checked", core);
		return true;
	}

	bool r = true;

	MTY_MutexLock(ctx->m);

	if (ctx->core_hash) {
		const char *platform = csync_get_platform();
		const char *sha256 = MTY_JSONObjGetStringPtr(MTY_JSONObjGetItem(
			MTY_JSONObjGetItem(ctx->core_hash, platform), CSYNC_CORE_ARCH), core);

		if (sha256) {
			char hash[MTY_SHA256_HEX_MAX] = {0};
			if (MTY_CryptoHashFile(MTY_ALGORITHM_SHA256_HEX, file, NULL, 0, hash, MTY_SHA256_HEX_MAX))
				r = !strcmp(sha256, hash);
		}
	}

	MTY_MutexUnlock(ctx->m);

	return r;
}


// Main

static void csync_simple_command(MTY_Queue *q, enum csync_cmd_type type)
{
	struct csync_cmd *cmd = MTY_QueueGetInputBuffer(q, sizeof(struct csync_cmd));

	if (cmd) {
		cmd->type = type;
		MTY_QueuePush(q);
	}
}

static void *csync_thread(void *opaque)
{
	struct csync *ctx = opaque;

	for (bool done = false; !done;) {
		struct csync_cmd *cmd = NULL;
		if (!MTY_QueueGetOutputBuffer(ctx->q, -1, (void **) &cmd, NULL))
			continue;

		switch (cmd->type) {
			case CSYNC_CMD_STOP:
				done = true;
				break;
			case CSYNC_CMD_FETCH_CORE:
				csync_fetch_core_cmd(ctx, cmd);
				break;
			case CSYNC_CMD_FETCH_CORE_HASH:
				csync_fetch_core_hash_cmd(ctx, cmd);
				break;
			default:
				break;
		}

		MTY_QueuePop(ctx->q);
	}

	return NULL;
}

struct csync *csync_start(void)
{
	struct csync *ctx = MTY_Alloc(1, sizeof(struct csync));

	ctx->q = MTY_QueueCreate(20);
	ctx->m = MTY_MutexCreate();

	ctx->thread = MTY_ThreadCreate(csync_thread, ctx);

	csync_simple_command(ctx->q, CSYNC_CMD_FETCH_CORE_HASH);

	return ctx;
}

void csync_stop(struct csync **csync)
{
	if (!csync || !*csync)
		return;

	struct csync *ctx = *csync;

	if (ctx->q)
		csync_simple_command(ctx->q, CSYNC_CMD_STOP);

	MTY_ThreadDestroy(&ctx->thread);
	MTY_MutexDestroy(&ctx->m);
	MTY_QueueDestroy(&ctx->q);

	MTY_JSONDestroy(&ctx->core_hash);

	MTY_Free(ctx);
	*csync = NULL;
}
