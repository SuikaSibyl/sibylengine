#include <se.gfx.hpp>
#include "se.editor.helper.hpp"

namespace se {
namespace gfx{
  auto MaterialInterpreterManager::init(Material* mat, int32_t type_id) noexcept -> void {
    Singleton<MaterialInterpreterManager>::instance()->
      m_intepretors[type_id].initMat(mat);
  }

	auto MaterialInterpreterManager::draw_gui(Material* mat, int32_t type_id) noexcept -> void {
    if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_None)) {
      ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 100);
      ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch);

      // Draw Name
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Name");
      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
      char buffer[256];
      memset(buffer, 0, sizeof(buffer));
      strncpy(buffer, mat->m_name.c_str(), sizeof(buffer) - 1);
      buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination
      if (ImGui::InputText(" ", buffer, sizeof(buffer))) {
        mat->m_name = std::string(buffer);
        mat->m_dirtyToFile = true;
      }
      ImGui::PopItemWidth();

      // Draw Type
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("BxDF");
      ImGui::TableSetColumnIndex(1);
      ImGui::PushItemWidth(-FLT_MIN);
      int currentType = mat->m_packet.bxdfType;
      const char* currentLabel = Singleton<MaterialInterpreterManager>::instance()
        ->m_intepretors[currentType].name.c_str();
      if (ImGui::BeginCombo("##BXDF", currentLabel)) {
        for (auto& iter: Singleton<MaterialInterpreterManager>::instance()->m_intepretors) {
          bool isSelected = (iter.first == currentType);
          if (ImGui::Selectable(iter.second.name.c_str(), isSelected)) {
            mat->m_packet.bxdfType = iter.first;
            mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
            iter.second.setDefault(mat);
          }
          if (isSelected) {
            ImGui::SetItemDefaultFocus(); // Optional: focuses the selected item
          }
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();


      Singleton<MaterialInterpreterManager>::instance()->
        m_intepretors[type_id].drawGui(mat);

      ImGui::EndTable();
    }
	}
  
  auto DirectionalLights::init(Light* mat) noexcept -> void {

  }

  auto DirectionalLights::set_default(Light* mat) noexcept -> void {

  }
  
  auto DirectionalLights::draw_gui(Light* mat) noexcept -> void {
    // Draw Basecolor
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Albedo");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    //if (ImGui::ColorEdit3("##albedo", &(mat->m_packet.vec4Data0[0]),
    //  ImGuiColorEditFlags_NoAlpha)) {
    //  mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    //}
    ImGui::PopItemWidth();
  }

  auto LambertianMaterial::init(Material* mat) noexcept -> void {}
  
  auto LambertianMaterial::set_default(Material* mat) noexcept -> void {}

	auto LambertianMaterial::draw_gui(Material* mat) noexcept -> void {

    // Draw Basecolor
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Albedo");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##albedo", &(mat->m_packet.vec4Data0[0]),
      ImGuiColorEditFlags_NoAlpha)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    //ImGui::TableNextRow();
    //ImGui::TableSetColumnIndex(0);
    //ImGui::Text("Albedo Texture");
    //ImGui::TableSetColumnIndex(1);
    //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    //ImGui::ColorEdit3("##emission", &(mat->m_packet.vec4Data1[0]),
    //  ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
    //ImGui::PopItemWidth();

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Emission");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##emission", &(mat->m_packet.vec4Data1[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();
    
	}
}
}