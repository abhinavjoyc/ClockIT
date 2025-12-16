

// -------------------- OpenGL & Image ----------------------
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "image.h"

// -------------------- SDL -------------------------------
#include <SDL.h>
#include <SDL_opengl.h>

// -------------------- ImGui -----------------------------
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// -------------------- Networking & JSON -----------------
#include "httplib.h"
#include "json.hpp"

// -------------------- STL -------------------------------
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <ctime>

// -------------------- Audio -----------------------------
#include <SDL_mixer.h>
#include "audio.h"

// -------------------- App Modules -----------------------
#include "settings.h"
#include "Weather.h"
#include "pomedoro.h"

using json = nlohmann::json;

// ==========================================================
// TEXTURE LOADER (OpenGL)
// ==========================================================
static GLuint LoadTexture(const char* filename)
{
    if (!filename) {
        std::cerr << "[Texture] filename is null\n";
        return 0;
    }

    int width = 0, height = 0, channels = 0;
    unsigned char* data =
        stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        std::cerr << "[Texture] Failed to load " << filename << "\n";
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "[Texture] Loaded " << filename
        << " (" << width << "x" << height << ")\n";

    return tex;
}

// ==========================================================
// ROOT WINDOW 
// ==========================================================
static void BeginRoot(ImGuiIO& io)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("##ROOT", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );
}

// ==========================================================
// TOP TAB BAR
// ==========================================================
void RenderCustomTabs(int& activeTab)
{
    // Rounded buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 12));

    // Button colors
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.85f, 0.85f, 0.30f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 0.90f));

    // Center tabs
    float width = ImGui::GetWindowWidth();
    ImGui::SetCursorPosX((width - 360) * 0.5f);

    // ---------------- Pomodoro ----------------
    ImGui::PushStyleColor(ImGuiCol_Text,
        activeTab == 0 ? ImVec4(0, 0, 0, 1) : ImVec4(0.4f, 0.4f, 0.4f, 1));

    if (ImGui::Button("Pomodoro", ImVec2(110, 40)))
        activeTab = 0;

    ImGui::PopStyleColor();
    ImGui::SameLine();

    // ---------------- Weather -----------------
    ImGui::PushStyleColor(ImGuiCol_Text,
        activeTab == 1 ? ImVec4(0, 0, 0, 1) : ImVec4(0.4f, 0.4f, 0.4f, 1));

    if (ImGui::Button("Weather", ImVec2(110, 40)))
        activeTab = 1;

    ImGui::PopStyleColor();
    ImGui::SameLine();

    // ---------------- Settings ----------------
    ImGui::PushStyleColor(ImGuiCol_Text,
        activeTab == 2 ? ImVec4(0, 0, 0, 1) : ImVec4(0.4f, 0.4f, 0.4f, 1));

    if (ImGui::Button("Settings", ImVec2(110, 40)))
        activeTab = 2;

    ImGui::PopStyleColor();

    // Cleanup
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

// ==========================================================
// MAIN ENTRY POINT
// ==========================================================
int main(int, char**)
{
    // ---------------- SDL Init ----------------
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL Init failed\n";
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window* window = SDL_CreateWindow(
        "Weather UI",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---------------- Audio Init --------------
    if (!Audio_Init()) return 1;

    Mix_Chunk* rain = Audio_LoadSfx("assets/audio/rain.wav");
    Mix_Chunk* alarm = Audio_LoadSfx("assets/audio/alarm.wav");

    std::vector<Mix_Chunk*> audiofiles{ rain, alarm };

    // ---------------- ImGui Init --------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ---------------- Resources ----------------
    GLuint bgtex = LoadTexture("assets/images/background.jpg");
    GLuint icontex = LoadTexture("assets/images/thunderstorm.png");
    GLuint clockTex = LoadTexture("assets/images/stopwatch.png");
    GLuint arrowTex = LoadTexture("assets/images/arrow.jpg");
    GLuint startTex = LoadTexture("assets/images/start.png");
    GLuint stopTex = LoadTexture("assets/images/stop.png");
    GLuint pauseTex = LoadTexture("assets/images/pause.png");
    GLuint resetTex = LoadTexture("assets/images/reset.png");

    io.Fonts->AddFontDefault();
    ImFont* bigFont = io.Fonts->AddFontFromFileTTF(
        "assets/fonts/ScienceGothic-Medium.ttf",
        8.0f
    );

    std::vector<GLuint> textures{
        bgtex, icontex, clockTex, arrowTex,
        startTex, stopTex, pauseTex, resetTex
    };

    // ---------------- App State ----------------
    bool running = true;
    SDL_Event e;
    int activeTab = 0;

    // ======================================================
    // MAIN LOOP
    // ======================================================
    while (running)
    {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // -------- Root Content --------
        BeginRoot(io);

        ImGui::SetWindowFontScale(6.0f);

        if (activeTab == 0)
            PomederoTab(io, textures, bigFont, audiofiles);
        else if (activeTab == 1)
            weathertab(io, textures, bigFont);
        else
            Settingtab(io, textures, bigFont, audiofiles);

        // -------- Top Tabs (drawn LAST) --------
        ImGui::SetNextWindowPos(ImVec2(0, 6), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 80), ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 6));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

        ImGui::Begin("Tabs", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoFocusOnAppearing
        );

        RenderCustomTabs(activeTab);

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::End(); // END ROOT

        // -------- Render --------
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();
    return 0;
}
