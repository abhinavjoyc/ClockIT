#pragma once

#include <vector>
#include <glad/glad.h>
#include "imgui.h"

// Draws UI with background texture + centered text + ON/OFF button
void Settingtab(ImGuiIO& io, std::vector<GLuint>& textures, ImFont* bigFont, std::vector<Mix_Chunk*> audiofiles);
