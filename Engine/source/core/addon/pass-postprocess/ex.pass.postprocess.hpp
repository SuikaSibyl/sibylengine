#pragma once
#include "se.rdg.hpp"
#include "../../source/se.editor.helper.hpp"

using namespace se;

struct AccumulatePass :public rdg::ComputePass {
	AccumulatePass();
	virtual auto reflect(rdg::PassReflection& reflect) noexcept -> rdg::PassReflection override;
	virtual auto render_ui() noexcept -> void override;
	virtual auto execute(rdg::RenderContext* context, rdg::RenderData const& renderData) noexcept -> void;

	struct PushConstant {
		se::uvec2 resolution;
		uint32_t gAccumCount = 0;
		uint32_t gAccumulate = 0;
		uint32_t gMovingAverageMode;
	} pConst;
	int maxAccumCount = 5;
	ivec3 resolution;
};

AccumulatePass::AccumulatePass() {
	init("./shaders/passes/accumulate.slang");
}

auto AccumulatePass::reflect(rdg::PassReflection& reflector) noexcept -> rdg::PassReflection {
	reflector.add_output("Output").is_texture()
		.with_format(rhi::TextureFormat::RGBA32_FLOAT)
		.consume_as_storage_binding_in_compute();
	reflector.add_input("Input").is_texture()
		.with_format(rhi::TextureFormat::RGBA32_FLOAT)
		.consume_as_storage_binding_in_compute();
	reflector.add_internal("LastSum").is_texture()
		.with_format(rhi::TextureFormat::RGBA32_FLOAT)
		.consume_as_storage_binding_in_compute();
	return reflector;
}

auto AccumulatePass::render_ui() noexcept -> void {
	ImGui::DragInt("Max Accum", &maxAccumCount, 1, 0);
	if (ImGui::Button("Reset")) {
		pConst.gAccumCount = 0;
	}
	ImGui::SameLine();
	bool useAccum = pConst.gAccumulate != 0;
	if (ImGui::Checkbox("Use Accum", &useAccum)) {
		pConst.gAccumulate = useAccum ? 1 : 0;
	}
	//pConst.gAccumulate = useAccum ? 1 : 0;
	ImGui::Text(std::string("Accumulated Count: " + std::to_string(pConst.gAccumCount)).c_str());
}

auto AccumulatePass::execute(rdg::RenderContext* context, rdg::RenderData const& renderData) noexcept -> void {
	gfx::TextureHandle output = renderData.get_texture("Output");
	gfx::TextureHandle input = renderData.get_texture("Input");
	gfx::TextureHandle sum = renderData.get_texture("LastSum");

	update_bindings(context, {
		{"u_input", rhi::BindingResource{input->get_uav(0,0,1)}},
		{"u_lastSum", rhi::BindingResource{sum->get_uav(0,0,1)}},
		{"u_output", rhi::BindingResource{output->get_uav(0,0,1)}},
		});

	uint32_t width = input->m_texture->width();
	uint32_t height = input->m_texture->height();
	uint32_t batchIdx = 1;
	pConst.resolution = se::uvec2{ width,height };
	if (batchIdx == 0) pConst.gAccumCount = 0;
	pConst.gMovingAverageMode = pConst.gAccumCount > 0 ? 1 : 0;

	se::rhi::ComputePassEncoder* encoder = begin_pass(context);
	encoder->push_constants(&pConst, (uint32_t)rhi::ShaderStageEnum::COMPUTE, 0, sizeof(pConst));
	encoder->dispatch_workgroups((width + 15) / 16, (height + 15) / 16, 1);
	encoder->end();

	if (maxAccumCount == 0 || pConst.gAccumCount < maxAccumCount) pConst.gAccumCount++;
}