#pragma once

#include <string>

void PatchGameMenuLayoutFunction();

bool LoadGameMenuLayout(const std::string &filepath);

void PatchMenuCustomLayoutDefault();

void SetMainMenuLayoutLoadCallback();