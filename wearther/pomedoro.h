#pragma once
#include <glad/glad.h>

#include "Weather.h"




#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"


#include <SDL_mixer.h>

#include "httplib.h"
#include "json.hpp"

#include <iostream>
#include <string>
#include <sstream>


#include<chrono>
#include<ctime>



void PomederoTab(ImGuiIO& io, std::vector<GLuint>& textures, ImFont* bigFont , std::vector<Mix_Chunk*> audiofiles);
ImVec2 GetScaledSizeFromGLTexture(GLuint glTex, const ImVec2& maxBox);
ImVec2 GetRawTexSize(GLuint tex);
