#include "se.bxdf.microfacet.hpp"

namespace se {
namespace gfx {
  auto PlasticMaterial::init(Material* mat) noexcept -> void {

  }

  auto PlasticMaterial::set_default(Material* mat) noexcept -> void {

  }

  auto PlasticMaterial::draw_gui(Material* mat) noexcept -> void {
    // Draw Basecolor
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Kd");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##kd", &(mat->m_packet.vec4Data0[0]),
      ImGuiColorEditFlags_NoAlpha)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw Kappa
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Ks");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##ks", &(mat->m_packet.vec4Data2[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Alpha");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##alpha", &mat->m_packet.vec4Data1[3], 0.05, 0.0, 1.0)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Eta");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##eta", &mat->m_packet.vec4Data2[3], 0.05, 0.0, 1.0)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();
  }

  auto ConductorMaterial::init(Material* mat) noexcept -> void {

  }

  auto ConductorMaterial::set_default(Material* mat) noexcept -> void {

  }

  auto ConductorMaterial::draw_gui(Material* mat) noexcept -> void {
    // Draw Kappa
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Kappa");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##kappa", &(mat->m_packet.vec4Data0[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Alpha");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##alpha", &mat->m_packet.vec4Data1[3], 0.05, 0.0, 1.0)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw eta
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Eta");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##eta", &(mat->m_packet.vec4Data2[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();
  }

  auto DielectricMaterial::init(Material* mat) noexcept -> void {

  }

  auto DielectricMaterial::set_default(Material* mat) noexcept -> void {

  }

  auto DielectricMaterial::draw_gui(Material* mat) noexcept -> void {
    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Eta");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##eta", &mat->m_packet.vec4Data1[3], 0.05, 0.0, 1.0)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Alpha");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##alpha", &mat->m_packet.vec4Data2[3], 0.05, 0.0, 1.0)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();
  }

  auto ChromaGGXMaterial::init(Material* mat) noexcept -> void {

  }

  auto ChromaGGXMaterial::set_default(Material* mat) noexcept -> void {

  }

  auto ChromaGGXMaterial::draw_gui(Material* mat) noexcept -> void {
    // Draw Kappa
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Kappa");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##kappa", &(mat->m_packet.vec4Data0[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();

    // Draw alpha
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Alpha");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    vec3 alpha_rgb = {
      mat->m_packet.vec4Data0[3],
      mat->m_packet.vec4Data1[3],
      mat->m_packet.vec4Data2[3],
    };
    if (ImGui::ColorEdit3("##eta", (float*)&alpha_rgb,
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
      mat->m_packet.vec4Data0[3] = alpha_rgb[0];
      mat->m_packet.vec4Data1[3] = alpha_rgb[1];
      mat->m_packet.vec4Data2[3] = alpha_rgb[2];
    }
    ImGui::PopItemWidth();

    // Draw eta
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Eta");
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::ColorEdit3("##eta", &(mat->m_packet.vec4Data2[0]),
      ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float)) {
      mat->m_dirtyToFile = mat->m_dirtyToGPU = true;
    }
    ImGui::PopItemWidth();
  }
}
}