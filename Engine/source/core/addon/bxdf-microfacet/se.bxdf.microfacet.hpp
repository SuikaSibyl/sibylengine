#pragma once
#include <se.gfx.hpp>

namespace se {
namespace gfx {
  struct ConductorMaterial {
    static auto init(Material* mat) noexcept -> void;
    static auto set_default(Material* mat) noexcept -> void;
    static auto draw_gui(Material* mat) noexcept -> void;
  };

  struct DielectricMaterial {
    static auto init(Material* mat) noexcept -> void;
    static auto set_default(Material* mat) noexcept -> void;
    static auto draw_gui(Material* mat) noexcept -> void;
  };

  struct PlasticMaterial {
    static auto init(Material* mat) noexcept -> void;
    static auto set_default(Material* mat) noexcept -> void;
    static auto draw_gui(Material* mat) noexcept -> void;
  };

  struct ChromaGGXMaterial {
    static auto init(Material* mat) noexcept -> void;
    static auto set_default(Material* mat) noexcept -> void;
    static auto draw_gui(Material* mat) noexcept -> void;
  };
}
}