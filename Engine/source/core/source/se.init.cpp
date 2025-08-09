#include <se.utils.hpp>
#include <se.gfx.hpp>
#include <se.editor.hpp>
#include "../addon/bxdf-microfacet/se.bxdf.microfacet.hpp"
#include "../addon/bxdf-rgl/se.bxdf.rglbrdf.hpp"

namespace se {
	auto init_extensions() noexcept -> void {
		// register components
		gfx::ComponentManager::registrate<gfx::NodeProperty>(-1, "Node",					false);
		gfx::ComponentManager::registrate<gfx::Transform>		( 0, "Transform",			false);
		gfx::ComponentManager::registrate<gfx::MeshRenderer>( 1, "Mesh Renderer", true);
		gfx::ComponentManager::registrate<gfx::Light>				( 2, "Light",					true);
		gfx::ComponentManager::registrate<gfx::Camera>			( 3, "Camera",				true);
		gfx::ComponentManager::registrate<gfx::Script>			(10, "Script",				true);

		// register predefined materials
		gfx::MaterialInterpreterManager::registrate<gfx::LambertianMaterial>	(0, "Lambertian Material");
		gfx::MaterialInterpreterManager::registrate<gfx::ConductorMaterial>		(1, "Conductor Material");
		gfx::MaterialInterpreterManager::registrate<gfx::DielectricMaterial>	(2, "Dielectric Material");
		gfx::MaterialInterpreterManager::registrate<gfx::PlasticMaterial>			(3, "Plastic Material");
		gfx::MaterialInterpreterManager::registrate<gfx::ChromaGGXMaterial>		(9, "ChromaGGX Material");
		gfx::MaterialInterpreterManager::registrate<gfx::RGLBrdfMaterial>			(10, "RGL Material");

		// register predefined lights
		gfx::LightInterpreterManager::registrate<gfx::DirectionalLights>(0, "Directional Light");


		// register predefined scripts
		gfx::ScriptManager::registrate<editor::EditorCameraControllerScript>("EditorCameraControllerScript");
	}
}