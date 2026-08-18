#pragma once
#include "Rose/Graphics/Shader.hpp"

#include "Platform/Vulkan/Graphics/VulkanTexture.hpp"
#include "Platform/Vulkan/VulkanInclude.hpp"

#include <spirv_cross.hpp>

namespace Rose {
    struct ShaderResource
    {
        std::string Name;
        vk::DescriptorType Type;

        U32 Set = U32Limit;
        U32 Binding = U32Limit;

        vk::DeviceSize Size;

        vk::ShaderStageFlags Stages;

        ShaderResourceKey Key() const { return {Set, Binding}; }
    };

    enum class ShaderStage : U8
    {
        Vertex,
        Fragment
    };

    struct ShaderSources
    {
        std::string VertexShader;
        std::string FragmentShader;
    };

    class VulkanShader : public Shader
    {
    public:
        explicit VulkanShader(std::string&& path);
        ~VulkanShader() override;

        void Bind() const override;
        void Reload() override;

        void SetUBO(ShaderResourceKey key, const void* ubo, U64 size) override;
        void SetTexture(ShaderResourceKey key, const Ref<Texture>& texture) override;

    private:
        static std::vector<U32> Compile(const std::string& path, ShaderStage stage, const std::string& code);
        static ShaderSources PreProcess(const std::string& code);

        void DestroyResources();

        void CreateDescriptors();
        void CreateUniformObjects();
        void CreateGraphicsPipeline(std::vector<U32>&& vertShaderBytecode, std::vector<U32>&& fragShaderBytecode);

        void BindDescriptorSet(U32 set) const;

    private:
        bool m_Loaded = false;

        vk::Pipeline m_Pipeline;
        vk::PipelineLayout m_Layout;

        std::map<ShaderResourceKey, ShaderResource> m_Resources;
        vk::DescriptorPool m_DescriptorPool;
        std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
        std::vector<std::vector<vk::DescriptorSet>> m_DescriptorSets;

        std::map<ShaderResourceKey, std::vector<vk::Buffer>> m_UniformBuffers;
        std::map<ShaderResourceKey, std::vector<vk::DeviceMemory>> m_UniformBuffersMemory;
        std::map<ShaderResourceKey, std::vector<void*>> m_UniformBuffersMapped;
    };
} // namespace Rose
