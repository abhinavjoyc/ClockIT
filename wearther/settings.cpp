#include "pomedoro.h"
#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_opengl.h>
#include <glad/glad.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "audio.h"

// Layout constants
static constexpr float TOGGLE_WIDTH = 50.0f;
static constexpr float SLIDER_WIDTH = 300.0f;
static constexpr float CONTROL_HEIGHT = 26.0f;
static constexpr float LABEL_OFFSET_X = 240.0f;

// ============================================================================
// TOGGLE (BLACK WHEN ON)
// ============================================================================
void GlassToggle(const char* id, const char* label, bool* value)
{
    ImGui::PushID(id);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::SameLine(LABEL_OFFSET_X);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float h = CONTROL_HEIGHT;
    float w = TOGGLE_WIDTH;
    float r = h * 0.5f;

    ImGui::InvisibleButton("toggle", ImVec2(w, h));

    if (ImGui::IsItemClicked())
    {
        *value = !(*value);
        std::cout << *value;
        std::cout << label << ": " << (*value ? "ON" : "OFF") << std::endl;
        if (*value)
        {
            Audio_UnmuteAll();
        }
        else
        {
            Audio_MuteAll();
        }
    }

    dl->AddRectFilled(
        pos,
        ImVec2(pos.x + w, pos.y + h),
        *value ? IM_COL32(0, 0, 0, 255)
        : IM_COL32(220, 220, 220, 255),
        r
    );

    float knobX = *value ? (pos.x + w - r) : (pos.x + r);

    dl->AddCircleFilled(
        ImVec2(knobX, pos.y + r + 1),
        r * 0.65f,
        IM_COL32(0, 0, 0, 60)
    );

    dl->AddCircleFilled(
        ImVec2(knobX, pos.y + r),
        r * 0.6f,
        IM_COL32(255, 255, 255, 255)
    );

    ImGui::PopID();
}

// ============================================================================
// SLIDER (MANUAL DRAWING - GUARANTEED STRAIGHT EDGE TO BUTTON)
// ============================================================================
void GlassSliderFloat(
    const char* id,
    const char* label,
    float* value,
    float min,
    float max)
{
    ImGui::PushID(id);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::SameLine(LABEL_OFFSET_X);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float w = SLIDER_WIDTH;
    float h = CONTROL_HEIGHT;
    float r = h * 0.5f;

    ImGui::InvisibleButton("slider", ImVec2(w, h));

    if (ImGui::IsItemActive())
    {
        float t = (ImGui::GetIO().MousePos.x - pos.x) / w;
        t = t < 0 ? 0 : (t > 1 ? 1 : t);
        *value = min + (max - min) * t;
    }

    if (ImGui::IsItemDeactivated())
    {
        std::cout << label << ": " << *value << std::endl;
        int x = *value * 100;
        Audio_SetAllChannelVolume(x);
        std::cout << "volume changed";
        
    }

    float t = (*value - min) / (max - min);
    float knobRadius = r * 0.85f;
    float knobPos = w * t;
    float knobCenterX = pos.x + knobPos;
    float knobCenterY = pos.y + r;

    // Track background (full rounded rectangle)
    dl->AddRectFilled(
        pos,
        ImVec2(pos.x + w, pos.y + h),
        IM_COL32(220, 220, 220, 255),
        r
    );

    // Black fill - MANUAL CONSTRUCTION: rounded left + straight rectangle
    if (t > 0.001f)
    {
        float fillEnd = knobCenterX;
        
        if (fillEnd - pos.x > r)
        {
            // Draw left rounded cap (semicircle)
            dl->AddCircleFilled(
                ImVec2(pos.x + r, pos.y + r),
                r,
                IM_COL32(0, 0, 0, 255)
            );
            
            // Draw straight rectangle from left cap to button center
            dl->AddRectFilled(
                ImVec2(pos.x + r, pos.y),
                ImVec2(fillEnd, pos.y + h),
                IM_COL32(0, 0, 0, 255),
                0.0f  // NO ROUNDING!
            );
        }
        else
        {
            // Very small fill
            dl->AddCircleFilled(
                ImVec2(pos.x + r, pos.y + r),
                (fillEnd - pos.x),
                IM_COL32(0, 0, 0, 255)
            );
        }
    }

    // Knob shadow
    dl->AddCircleFilled(
        ImVec2(knobCenterX, knobCenterY + 1),
        knobRadius,
        IM_COL32(0, 0, 0, 80)
    );

    // Knob (white circle - covers the straight edge)
    dl->AddCircleFilled(
        ImVec2(knobCenterX, knobCenterY),
        knobRadius * 0.95f,
        IM_COL32(255, 255, 255, 255)
    );

    ImGui::PopID();
}

// ============================================================================
// DROPDOWN / SELECTION LIST
// ============================================================================
void GlassCombo(
    const char* label,
    const char** items,
    int itemCount,
    int* current)
{
    ImGui::PushID(label);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::SameLine(LABEL_OFFSET_X);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(220, 220, 220, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(200, 200, 200, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(180, 180, 180, 255));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(220, 220, 220, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 200, 200, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(180, 180, 180, 255));
    ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(40, 40, 40, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(60, 60, 60, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(240, 240, 240, 255));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, CONTROL_HEIGHT * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);

    ImGui::SetNextItemWidth(200);

    int oldSelection = *current;
    if (ImGui::BeginCombo("##combo", items[*current]))
    {
        for (int i = 0; i < itemCount; i++)
        {
            bool selected = (*current == i);

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

            if (ImGui::Selectable(items[i], selected))
            {
                *current = i;
            }

            if (selected)
            {
                ImGui::PopStyleColor();
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (*current != oldSelection)
    {
        std::cout << label << ": " << items[*current] << std::endl;
      

    }

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(11);
    ImGui::PopID();
}

// ============================================================================
// SETTINGS TAB
// ============================================================================
void Settingtab(
    ImGuiIO& io,
    std::vector<GLuint>& textures,
    ImFont* bigFont,
    std::vector<Mix_Chunk*> audiofiles
)
{
    // Background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("BG", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing);

    if (!textures.empty())
        ImGui::Image((ImTextureID)(intptr_t)textures[0], io.DisplaySize);

    ImGui::End();
    ImGui::PopStyleVar(2);

    // Settings Panel
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(ImVec2(center.x - 320, center.y - 180));
    ImGui::SetNextWindowSize(ImVec2(640, 360));

    ImGui::Begin("SettingsPanel", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize);

    // Heading
    ImGui::PushFont(bigFont);
    ImGui::SetCursorPosX(
        (ImGui::GetWindowSize().x -
            ImGui::CalcTextSize("SETTINGS").x) * 0.5f);
    ImGui::Text("SETTINGS");
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0, 35));

    static bool  rainEnabled = true;
    static float rainVolume = 1.0f;
    static int   rainPreset = 0;

    const char* presets[] = { "rain", "wave", "wind" };

    GlassToggle("rain_toggle", "Sound", &rainEnabled);
    ImGui::Dummy(ImVec2(0, 24));

    GlassSliderFloat("System Volume", "Rain Volume", &rainVolume, 0.0f, 1.0f);
    ImGui::Dummy(ImVec2(0, 24));

    GlassCombo("Voice List", presets, 3, &rainPreset);

    ImGui::End();
    ImGui::PopStyleVar(2);
}
