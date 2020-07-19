#pragma once
#include <ak/ak.hpp>

namespace ak
{
	struct shader_group_info
	{
		/** Number of shader records in this group */
		size_t mNumEntries;
		/** Entry-offset (not byte-offset) within the Shader Binding Table to the start of this group */
		vk::DeviceSize mOffset;
		/** Byte-offset (not entry-offset) within the Shader Binding Table to the start of this group */
		vk::DeviceSize mByteOffset;
	};
	
	struct shader_binding_table_groups_info
	{
		std::vector<shader_group_info> mRaygenGroupsInfo;
		std::vector<shader_group_info> mMissGroupsInfo;
		std::vector<shader_group_info> mHitGroupsInfo;
		std::vector<shader_group_info> mCallableGroupsInfo;
		vk::DeviceSize mEndOffset;
		vk::DeviceSize mTotalSize;
	};
	
	/** Represents data for a vulkan ray tracing pipeline */
	class ray_tracing_pipeline_t
	{
		friend class root;
		
	public:
		ray_tracing_pipeline_t() = default;
		ray_tracing_pipeline_t(ray_tracing_pipeline_t&&) noexcept = default;
		ray_tracing_pipeline_t(const ray_tracing_pipeline_t&) = delete;
		ray_tracing_pipeline_t& operator=(ray_tracing_pipeline_t&&) noexcept = default;
		ray_tracing_pipeline_t& operator=(const ray_tracing_pipeline_t&) = delete;
		~ray_tracing_pipeline_t();

		const auto& layout_handle() const { return mPipelineLayout.get(); }
		std::tuple<const ray_tracing_pipeline_t*, const vk::PipelineLayout, const std::vector<vk::PushConstantRange>*> layout() const { return std::make_tuple(this, layout_handle(), &mPushConstantRanges); }
		const auto& handle() const { return mPipeline.mHandle; }
		vk::DeviceSize table_offset_size() const { return static_cast<vk::DeviceSize>(mShaderGroupBaseAlignment); }
		vk::DeviceSize table_entry_size() const { return static_cast<vk::DeviceSize>(mShaderGroupHandleSize); }
		vk::DeviceSize table_size() const { return static_cast<vk::DeviceSize>(mShaderBindingTable->meta_at_index<buffer_meta>(0).total_size()); }
		const auto& shader_binding_table_handle() const { return mShaderBindingTable->buffer_handle(); }
		const auto& shader_binding_table_groups() const { return mShaderBindingTableGroupsInfo; }
		
	private:
		// TODO: What to do with flags?
		vk::PipelineCreateFlags mPipelineCreateFlags;

		// Our precious GPU shader programs:
		std::vector<shader> mShaders;
		std::vector<vk::PipelineShaderStageCreateInfo> mShaderStageCreateInfos;

		// Shader table a.k.a. shader groups:
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> mShaderGroupCreateInfos;

		// Info about the groups in the shader binding table:
		shader_binding_table_groups_info mShaderBindingTableGroupsInfo;

		// Maximum recursion depth:
		uint32_t mMaxRecursionDepth;

		// TODO: What to do with the base pipeline index?
		int32_t mBasePipelineIndex;

		// Pipeline layout, i.e. resource bindings
		set_of_descriptor_set_layouts mAllDescriptorSetLayouts;
		std::vector<vk::PushConstantRange> mPushConstantRanges;
		vk::PipelineLayoutCreateInfo mPipelineLayoutCreateInfo;

		// Handles:
		vk::UniquePipelineLayout mPipelineLayout;
		//vk::ResultValueType<vk::UniqueHandle<vk::Pipeline, vk::DispatchLoaderDynamic>>::type mPipeline;
		ak::handle_wrapper<vk::Pipeline> mPipeline;

		uint32_t mShaderGroupBaseAlignment;
		uint32_t mShaderGroupHandleSize;
		buffer mShaderBindingTable; // TODO: support more than one shader binding table?
	};

	using ray_tracing_pipeline = ak::owning_resource<ray_tracing_pipeline_t>;
	
	template <>
	inline void command_buffer_t::bind_pipeline<ray_tracing_pipeline_t>(const ray_tracing_pipeline_t& aPipeline)
	{
		handle().bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, aPipeline.handle());
	}

	template <>
	inline void command_buffer_t::bind_pipeline<ray_tracing_pipeline>(const ray_tracing_pipeline& aPipeline)
	{
		bind_pipeline<ray_tracing_pipeline_t>(aPipeline);
	}

	template <>
	inline void command_buffer_t::bind_descriptors<std::tuple<const ray_tracing_pipeline_t*, const vk::PipelineLayout, const std::vector<vk::PushConstantRange>*>>
		(std::tuple<const ray_tracing_pipeline_t*, const vk::PipelineLayout, const std::vector<vk::PushConstantRange>*> aPipelineLayout, std::vector<descriptor_set> aDescriptorSets)
	{
		command_buffer_t::bind_descriptors(vk::PipelineBindPoint::eRayTracingKHR, std::get<const ray_tracing_pipeline_t*>(aPipelineLayout)->layout_handle(), std::move(aDescriptorSets));
	}

}