#pragma once
#include <se.math.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace helper {
  template <typename T, int N>
  inline bool DrawVecControl(const std::string& label, T& value, float resetValue = 0.0f, float speed = 0.1f, float columnWidth = 100.0f) {
    static_assert(N >= 2 && N <= 4, "Only supports vec2, vec3, vec4");

    bool changed = false;
    ImGui::PushID(label.c_str());

    // Calculate available width and distribute among components
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float componentWidth = (totalWidth - (N - 1) * ImGui::GetStyle().ItemSpacing.x) / N;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    const ImVec4 axisColors[4] = {
        ImVec4(0.8f, 0.1f, 0.15f, 1.0f), // X
        ImVec4(0.2f, 0.7f, 0.2f, 1.0f),  // Y
        ImVec4(0.2f, 0.4f, 1.0f, 1.0f),  // Z
        ImVec4(1.0f, 1.0f, 0.0f, 1.0f)   // W
    };

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    for (int i = 0; i < N; ++i) {
      ImGui::PushStyleColor(ImGuiCol_Text, axisColors[i]);
      std::string btnLabel = std::string(1, "XYZW"[i]);
      if (ImGui::Button(btnLabel.c_str(), buttonSize)) {
        value[i] = resetValue;
        changed = true;
      }
      ImGui::PopStyleColor();

      ImGui::SameLine();
      ImGui::SetNextItemWidth(componentWidth - buttonSize.x - ImGui::GetStyle().ItemSpacing.x);
      std::string dragLabel = "##" + btnLabel + label;
      changed |= ImGui::DragFloat(dragLabel.c_str(), &value[i], speed);

      if (i < N - 1) ImGui::SameLine();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();

    return changed;
  }

  // Aliases for specific types
  inline bool DrawVec2Control(const std::string& label, se::vec2& v, float reset = 0.0f, float speed = 0.1f, float width = 100.f) {
    return DrawVecControl<se::vec2, 2>(label, v, reset, speed, width);
  }

  inline bool DrawVec3Control(const std::string& label, se::vec3& v, float reset = 0.0f, float speed = 0.1f, float width = 100.f) {
    return DrawVecControl<se::vec3, 3>(label, v, reset, speed, width);
  }

  inline bool DrawVec4Control(const std::string& label, se::vec4& v, float reset = 0.0f, float speed = 0.1f, float width = 100.f) {
    return DrawVecControl<se::vec4, 4>(label, v, reset, speed, width);
  }

  inline float UniformRowHeight() {
    return ImGui::GetFrameHeightWithSpacing();
  }

  template <typename WidgetFn>
  inline bool DrawLabeledControl(const char* label, WidgetFn widget) {
    bool changed = false;

    ImGui::TableNextRow(ImGuiTableRowFlags_None, UniformRowHeight());
    ImGui::TableSetColumnIndex(0);

    ImGui::AlignTextToFramePadding();  // Vertically aligns text with widget
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);  // Fill remaining width
    changed |= widget();

    return changed;
  }
  inline void DrawColoredBox(const char* name, float boxSize, ImVec2 screenPos, ImU32 const& color) {
    ImGui::SetCursorScreenPos(screenPos);

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    if (ImGui::Button(name, ImVec2(boxSize, boxSize))) {
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Click for options");
    }
    ImGui::PopStyleColor(3);
  }

}