#pragma once

#include <string>

void PatchGameMenuLayoutFunction();

bool LoadGameMenuLayout(const std::string &filepath);

void PatchMenuCustomLayoutDefault(int plg_maj, int plg_min, int plg_patch);

void SetMainMenuLayoutLoadCallback();