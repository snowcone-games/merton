#pragma once

#include <stdbool.h>

Core *loader_load(const char *name, const char *system_dir, const char *save_dir);
void loader_reset(void);
