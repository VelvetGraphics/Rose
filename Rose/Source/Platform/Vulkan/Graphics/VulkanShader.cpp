#include "Platform/Vulkan/Graphics/VulkanShader.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"
#include "Platform/Vulkan/Graphics/VulkanTexture.hpp"
#include "Platform/Vulkan/VulkanCall.hpp"
#include "Rose/Core/Core.hpp"
#include "shaderc/shaderc.hpp"

namespace Rose {
    namespace {
        shaderc_shader_kind TranslateShaderStage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage::Vertex:
                    return shaderc_vertex_shader;
                case ShaderStage::Fragment:
                    return shaderc_fragment_shader;
            }

            ASSERT(false, "Invalid shader stage");
            std::abort();
        }

        ShaderStage ShaderStageFromString(const std::string& string)
        {
            if (string == std::string("Vertex"))
                return ShaderStage::Vertex;

            if (string == std::string("Fragment"))
                return ShaderStage::Fragment;

            ASSERT(false, "Invalid string for shader stage");
            std::abort();
        }

        vk::ShaderModule CreateShaderModule(const std::vector<U32>& bytecode)
        {
            vk::ShaderModuleCreateInfo createInfo = {};
            createInfo.codeSize = bytecode.size() * sizeof(U32);
            createInfo.pCode = bytecode.data();

            auto shaderModuleResult = VulkanAPI::Device().createShaderModule(createInfo);
            ASSERT(shaderModuleResult.result == vk::Result::eSuccess, "Failed to create shader module");
            return shaderModuleResult.value;
        }

        vk::Format TranslateShaderDataType(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType::Float2:
                    return vk::Format::eR32G32Sfloat;
                case ShaderDataType::Float3:
                    return vk::Format::eR32G32B32Sfloat;
                case ShaderDataType::Float4:
                    return vk::Format::eR32G32B32A32Sfloat;
                default:
                    ASSERT(false, "Not implemented shader data type");
            }

            std::abort();
        }

        std::vector<vk::VertexInputAttributeDescription>
        TranslateAttribDesc(const std::vector<VertexShaderAttribute>& attributes)
        {
            std::vector<vk::VertexInputAttributeDescription> vulkanAttributes;
            for (VertexShaderAttribute attrib : attributes)
            {
                auto& vulkanAttrib = vulkanAttributes.emplace_back();
                vulkanAttrib.binding = 0;
                vulkanAttrib.location = attrib.Location;
                vulkanAttrib.format = TranslateShaderDataType(attrib.DataType);
                vulkanAttrib.offset = attrib.Offset;
            }

            return vulkanAttributes;
        }

        std::pair<ShaderResourceKey, ShaderResource> CreateResource(const spirv_cross::Compiler& comp,
                                                                    const spirv_cross::Resource& resource,
                                                                    vk::DescriptorType type)
        {
            U32 set = comp.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
            U32 binding = comp.get_decoration(resource.id, spv::Decoration::DecorationBinding);

            size_t size = 0;
            if (type == vk::DescriptorType::eUniformBuffer)
                size = comp.get_declared_struct_size(comp.get_type(resource.base_type_id));

            return {{set, binding},
                    {.Name = resource.name, .Type = type, .Set = set, .Binding = binding, .Size = size, .Stages = {}}};
        }

        void FindResourcesFromStage(const spirv_cross::Compiler& comp, vk::ShaderStageFlags stage,
                                    const spirv_cross::ShaderResources& resources,
                                    std::map<ShaderResourceKey, ShaderResource>& result)
        {
            for (const auto& resource : resources.uniform_buffers)
            {
                std::pair<ShaderResourceKey, ShaderResource> reflected =
                        CreateResource(comp, resource, vk::DescriptorType::eUniformBuffer);
                auto [it, inserted] = result.try_emplace(reflected.first, reflected.second);

                if (!inserted)
                {
                    ASSERT(it->second.Type == reflected.second.Type, "Descriptor type differs between shader stages");

                    ASSERT(it->second.Size == reflected.second.Size,
                           "Uniform buffer size differs between shader stages");
                }

                it->second.Stages |= stage;
            }

            for (const auto& resource : resources.sampled_images)
            {
                std::pair<ShaderResourceKey, ShaderResource> reflected =
                        CreateResource(comp, resource, vk::DescriptorType::eCombinedImageSampler);

                auto [it, inserted] = result.try_emplace(reflected.first, reflected.second);

                if (!inserted)
                {
                    ASSERT(it->second.Type == reflected.second.Type, "Descriptor type differs between shader stages");

                    ASSERT(it->second.Size == reflected.second.Size,
                           "Uniform buffer size differs between shader stages");
                }

                it->second.Stages |= stage;
            }
        }

        std::map<ShaderResourceKey, ShaderResource> Reflect(const std::vector<U32>& vertSpirv,
                                                            const std::vector<U32>& fragSpirv)
        {
            std::map<ShaderResourceKey, ShaderResource> result;

            spirv_cross::Compiler vertComp(vertSpirv);
            spirv_cross::ShaderResources resources = vertComp.get_shader_resources();

            FindResourcesFromStage(vertComp, vk::ShaderStageFlagBits::eVertex, resources, result);

            spirv_cross::Compiler fragComp(fragSpirv);
            resources = fragComp.get_shader_resources();

            FindResourcesFromStage(fragComp, vk::ShaderStageFlagBits::eFragment, resources, result);

            return result;
        }
    } // namespace

    VulkanShader::VulkanShader(std::string&& path) : Shader(std::move(path)) { VulkanShader::Reload(); }

    VulkanShader::~VulkanShader() { DestroyResources(); }

    void VulkanShader::Bind() const
    {
        const vk::Pipeline pipeline = m_Pipeline;

        VulkanAPI::Submit([pipeline](vk::CommandBuffer cmdBuffer) {
            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        });

        for (U32 set = 0; set < m_DescriptorSets.size(); set++)
            BindDescriptorSet(set);
    }

    void VulkanShader::Reload()
    {
        if (m_Loaded)
        {
            DestroyResources();
        }

        std::ifstream file(m_Path, std::ios::ate | std::ios::binary);
        ASSERT(file.is_open(), std::string("Failed to open file at path: " + m_Path));

        S64 fileSize = file.tellg();
        ASSERT(fileSize >= 0, "Failed to determine shader file size");

        file.seekg(0);

        std::string code;
        code.resize(fileSize);

        file.read(code.data(), fileSize);
        file.close();

        ShaderSources sources = PreProcess(code);
        std::vector<U32> vertShaderBytecode = Compile(m_Path, ShaderStage::Vertex, sources.VertexShader);
        std::vector<U32> fragShaderBytecode = Compile(m_Path, ShaderStage::Fragment, sources.FragmentShader);

        m_Resources = Reflect(vertShaderBytecode, fragShaderBytecode);

        CreateUniformObjects();
        CreateDescriptors();
        CreateGraphicsPipeline(std::move(vertShaderBytecode), std::move(fragShaderBytecode));

        m_Loaded = true;
    }

    void VulkanShader::SetUBO(ShaderResourceKey key, const void* ubo, U64 size)
    {
        ASSERT(ubo, "UBO can NOT be nullptr");

        auto resourceIt = m_Resources.find(key);
        ASSERT(resourceIt != m_Resources.end(), "Shader resource does not exist");

        const ShaderResource& resource = resourceIt->second;
        ASSERT(resource.Type == vk::DescriptorType::eUniformBuffer, "Shader resource is not a uniform buffer");
        ASSERT(size == resource.Size, "Size does not match resource size");

        auto mappedIt = m_UniformBuffersMapped.find(key);
        ASSERT(mappedIt != m_UniformBuffersMapped.end(), "Uniform buffer has no mapped memory");

        const U32 frame = VulkanAPI::FrameIndex();
        ASSERT(frame < mappedIt->second.size(), "Invalid frame index");

        memcpy(mappedIt->second[frame], ubo, size);
    }

    ShaderSources VulkanShader::PreProcess(const std::string& code)
    {
        ShaderSources result;

        const char* token = "#Type";
        size_t tokenLength = strlen(token);
        size_t pos = code.find(token, 0);

        while (pos != std::string::npos)
        {
            size_t eol = code.find_first_of("\r\n", pos);
            ASSERT(eol != std::string::npos, "Syntax error");
            size_t begin = pos + tokenLength + 1;
            ShaderStage type = ShaderStageFromString(code.substr(begin, eol - begin));

            size_t nextLinePos = code.find_first_not_of("\r\n", eol);
            ASSERT(nextLinePos != std::string::npos, "Syntax error");

            pos = code.find(token, nextLinePos);

            switch (type)
            {
                case ShaderStage::Vertex:
                    result.VertexShader = (pos == std::string::npos) ? code.substr(nextLinePos)
                                                                     : code.substr(nextLinePos, pos - nextLinePos);
                    break;
                case ShaderStage::Fragment:
                    result.FragmentShader = (pos == std::string::npos) ? code.substr(nextLinePos)
                                                                       : code.substr(nextLinePos, pos - nextLinePos);
                    break;
            }
        }

        return result;
    }

    void VulkanShader::DestroyResources()
    {
        ASSERT(VulkanAPI::Device().waitIdle() == vk::Result::eSuccess, "Failed to wait for device idle");

        if (m_Pipeline)
        {
            VulkanAPI::Device().destroyPipeline(m_Pipeline);

            m_Pipeline = nullptr;
        }

        if (m_Layout)
        {
            VulkanAPI::Device().destroyPipelineLayout(m_Layout);

            m_Layout = nullptr;
        }

        m_DescriptorSets.clear();

        if (m_DescriptorPool)
        {
            VulkanAPI::Device().destroyDescriptorPool(m_DescriptorPool);

            m_DescriptorPool = nullptr;
        }

        for (vk::DescriptorSetLayout layout : m_DescriptorSetLayouts)
        {
            if (layout)
            {
                VulkanAPI::Device().destroyDescriptorSetLayout(layout);
            }
        }

        m_DescriptorSetLayouts.clear();

        for (const auto& memories : m_UniformBuffersMemory | std::views::values)
        {
            for (vk::DeviceMemory memory : memories)
            {
                if (memory)
                {
                    VulkanAPI::Device().unmapMemory(memory);
                }
            }
        }

        m_UniformBuffersMapped.clear();

        for (const auto& buffers : m_UniformBuffers | std::views::values)
        {
            for (vk::Buffer buffer : buffers)
            {
                if (buffer)
                {
                    VulkanAPI::Device().destroyBuffer(buffer);
                }
            }
        }

        m_UniformBuffers.clear();

        for (const auto& memories : m_UniformBuffersMemory | std::views::values)
        {
            for (vk::DeviceMemory memory : memories)
            {
                if (memory)
                {
                    VulkanAPI::Device().freeMemory(memory);
                }
            }
        }

        m_UniformBuffersMemory.clear();

        m_Resources.clear();

        m_Loaded = false;
    }

    void VulkanShader::CreateDescriptors()
    {
        if (m_Resources.empty())
            return;

        constexpr U32 frameCount = VulkanAPI::MaxFramesInFlight();

        U32 maxSet = 0;

        for (const auto& resource : m_Resources | std::views::values)
        {
            maxSet = std::max(maxSet, resource.Set);
        }

        const U32 setCount = maxSet + 1;

        std::vector<std::vector<const ShaderResource*>> resourcesBySet(setCount);

        for (const auto& resource : m_Resources | std::views::values)
        {
            resourcesBySet[resource.Set].push_back(&resource);
        }

        m_DescriptorSetLayouts.resize(setCount);

        for (U32 set = 0; set < setCount; ++set)
        {
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            bindings.reserve(resourcesBySet[set].size());

            for (const ShaderResource* resource : resourcesBySet[set])
            {
                vk::DescriptorSetLayoutBinding binding{};
                binding.binding = resource->Binding;
                binding.descriptorType = resource->Type;
                binding.descriptorCount = 1;
                binding.stageFlags = resource->Stages;

                bindings.emplace_back(binding);
            }

            vk::DescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.bindingCount = static_cast<U32>(bindings.size());

            layoutInfo.pBindings = bindings.empty() ? nullptr : bindings.data();

            auto layoutResult = VulkanAPI::Device().createDescriptorSetLayout(layoutInfo);

            ASSERT(layoutResult.result == vk::Result::eSuccess, "Failed to create descriptor set layout");

            m_DescriptorSetLayouts[set] = layoutResult.value;
        }

        std::map<vk::DescriptorType, U32> descriptorCounts;

        for (const auto& resource : m_Resources | std::views::values)
        {
            descriptorCounts[resource.Type] += frameCount;
        }

        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.reserve(descriptorCounts.size());

        for (const auto& [type, count] : descriptorCounts)
        {
            vk::DescriptorPoolSize poolSize{};
            poolSize.type = type;
            poolSize.descriptorCount = count;

            poolSizes.emplace_back(poolSize);
        }

        vk::DescriptorPoolCreateInfo poolInfo{};

        poolInfo.maxSets = setCount * frameCount;

        poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());

        poolInfo.pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data();

        auto poolResult = VulkanAPI::Device().createDescriptorPool(poolInfo);

        ASSERT(poolResult.result == vk::Result::eSuccess, "Failed to create descriptor pool");

        m_DescriptorPool = poolResult.value;

        m_DescriptorSets.resize(setCount);

        for (U32 set = 0; set < setCount; ++set)
        {
            std::vector<vk::DescriptorSetLayout> frameLayouts(frameCount, m_DescriptorSetLayouts[set]);

            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_DescriptorPool;
            allocInfo.descriptorSetCount = frameCount;
            allocInfo.pSetLayouts = frameLayouts.data();

            auto descriptorSetsResult = VulkanAPI::Device().allocateDescriptorSets(allocInfo);

            ASSERT(descriptorSetsResult.result == vk::Result::eSuccess, "Failed to allocate descriptor sets");

            m_DescriptorSets[set] = std::move(descriptorSetsResult.value);
        }

        for (const auto& resource : m_Resources | std::views::values)
        {
            ShaderResourceKey key = resource.Key();

            switch (resource.Type)
            {
                case vk::DescriptorType::eUniformBuffer: {
                    for (U32 frame = 0; frame < frameCount; ++frame)
                    {
                        vk::DescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = m_UniformBuffers.at(key).at(frame);

                        bufferInfo.offset = 0;
                        bufferInfo.range = resource.Size;

                        vk::WriteDescriptorSet write{};
                        write.dstSet = m_DescriptorSets.at(resource.Set).at(frame);

                        write.dstBinding = resource.Binding;
                        write.dstArrayElement = 0;
                        write.descriptorCount = 1;
                        write.descriptorType = resource.Type;
                        write.pBufferInfo = &bufferInfo;

                        VulkanAPI::Device().updateDescriptorSets(write, {});
                    }

                    break;
                }

                case vk::DescriptorType::eCombinedImageSampler: {
                    break;
                }

                default:
                    ASSERT(false, "Unsupported descriptor type");

                    std::abort();
            }
        }
    }

    void VulkanShader::CreateUniformObjects()
    {
        constexpr U32 frameCount = VulkanAPI::MaxFramesInFlight();

        for (const auto& resource : m_Resources | std::views::values)
        {
            ShaderResourceKey key = resource.Key();

            switch (resource.Type)
            {
                case vk::DescriptorType::eUniformBuffer: {
                    auto& buffers = m_UniformBuffers[key];
                    auto& memories = m_UniformBuffersMemory[key];
                    auto& mappedMemories = m_UniformBuffersMapped[key];

                    buffers.reserve(frameCount);
                    memories.reserve(frameCount);
                    mappedMemories.reserve(frameCount);

                    for (U32 frame = 0; frame < frameCount; frame++)
                    {
                        auto [buffer, memory] = VulkanCall::CreateBuffer(
                                resource.Size, vk::BufferUsageFlagBits::eUniformBuffer,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                        auto mapResult = VulkanAPI::Device().mapMemory(memory, 0, resource.Size);

                        ASSERT(mapResult.result == vk::Result::eSuccess, "Failed to map uniform buffer memory");

                        buffers.emplace_back(buffer);
                        memories.emplace_back(memory);
                        mappedMemories.emplace_back(mapResult.value);
                    }
                    break;
                }

                case vk::DescriptorType::eCombinedImageSampler: {
                    break;
                }

                default:
                    ASSERT(false, "Unsupported resource type");
            }
        }
    }

    void VulkanShader::CreateGraphicsPipeline(std::vector<U32>&& vertShaderBytecode,
                                              std::vector<U32>&& fragShaderBytecode)
    {
        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

        auto vertShaderModule = CreateShaderModule(vertShaderBytecode);
        auto fragShaderModule = CreateShaderModule(fragShaderBytecode);

        // Vertex shader
        shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";

        // Fragment shader
        shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::VertexInputBindingDescription bindingDesc = {};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(Vertex);
        bindingDesc.inputRate = vk::VertexInputRate::eVertex;

        auto attribDesc = TranslateAttribDesc(Vertex::GetAttribDescription());

        vk::PipelineVertexInputStateCreateInfo vertexInput;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = attribDesc.size();
        vertexInput.pVertexAttributeDescriptions = attribDesc.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        depthStencilState.depthTestEnable = vk::True;
        depthStencilState.depthWriteEnable = vk::True;
        depthStencilState.depthCompareOp = vk::CompareOp::eLess;
        depthStencilState.depthBoundsTestEnable = vk::False;
        depthStencilState.stencilTestEnable = vk::False;

        vk::Extent2D swapChainExtent = VulkanAPI::SwapChainExtent();
        vk::Viewport viewport{
                0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height),
                0.0f, 1.0f};
        vk::Rect2D scissor{vk::Offset2D{0, 0}, swapChainExtent};

        vk::PipelineViewportStateCreateInfo viewportState = {};
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.depthClampEnable = vk::False;
        rasterizer.rasterizerDiscardEnable = vk::False;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizer.depthBiasEnable = vk::False;
        rasterizer.lineWidth = 1.0f;

        vk::PipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.sampleShadingEnable = vk::False;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = vk::True;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.logicOpEnable = vk::False;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        vk::PipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.setLayoutCount = m_DescriptorSetLayouts.size();
        layoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 0;

        auto layoutResult = VulkanAPI::Device().createPipelineLayout(layoutInfo);
        ASSERT(layoutResult.result == vk::Result::eSuccess, "Failed to create pipeline layout");
        m_Layout = layoutResult.value;

        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain;

        auto& pipelineInfo = pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>();
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_Layout;
        pipelineInfo.renderPass = nullptr;

        vk::Format colorAttachmentFormat = VulkanAPI::SwapChainSurfaceFormat();

        auto& pipelineRendering = pipelineCreateInfoChain.get<vk::PipelineRenderingCreateInfo>();
        pipelineRendering.colorAttachmentCount = 1;
        pipelineRendering.pColorAttachmentFormats = &colorAttachmentFormat;
        pipelineRendering.depthAttachmentFormat = VulkanAPI::DepthFormat();

        auto pipelineResult = VulkanAPI::Device().createGraphicsPipeline(VulkanAPI::PipelineCache(), pipelineInfo);
        ASSERT(pipelineResult.result == vk::Result::eSuccess, "Failed to create graphics pipeline");
        m_Pipeline = pipelineResult.value;

        VulkanAPI::Device().destroyShaderModule(vertShaderModule);
        VulkanAPI::Device().destroyShaderModule(fragShaderModule);
    }

    void VulkanShader::BindDescriptorSet(U32 set) const
    {
        ASSERT(set < m_DescriptorSets.size(), "Descriptor set does not exist");

        U32 frame = VulkanAPI::FrameIndex();
        ASSERT(frame < m_DescriptorSets[set].size(), "Invalid frame index");

        vk::PipelineLayout layout = m_Layout;
        vk::DescriptorSet descriptorSet = m_DescriptorSets[set][frame];

        VulkanAPI::Submit([layout, descriptorSet, set](vk::CommandBuffer cmdBuffer) {
            cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, set, descriptorSet, {});
        });
    }

    // TODO: Texture indexing
    void VulkanShader::SetTexture(ShaderResourceKey key, const Ref<Texture>& texture)
    {
        VulkanTexture* vulkanTexture = reinterpret_cast<VulkanTexture*>(texture.Raw());

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = vulkanTexture->m_ImageView;
        imageInfo.sampler = vulkanTexture->m_Sampler;

        vk::WriteDescriptorSet write{};
        write.dstSet = m_DescriptorSets.at(key.first).at(VulkanAPI::FrameIndex());
        write.dstBinding = key.second;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        write.pImageInfo = &imageInfo;

        VulkanAPI::Device().updateDescriptorSets(write, {});
    }

    std::vector<U32> VulkanShader::Compile(const std::string& path, ShaderStage stage, const std::string& code)
    {
        static std::unordered_map<std::string, std::pair<ShaderStage, std::vector<U32>>> s_Cached;
        auto it = s_Cached.find(code);
        if (it != s_Cached.end() && it->second.first == stage)
        {
            return it->second.second;
        }

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc::CompilationResult result =
                compiler.CompileGlslToSpv(code, TranslateShaderStage(stage), path.c_str(), options);

        ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success,
               std::string("Failed to compile shader to assembly, message: " + result.GetErrorMessage()));

        std::vector<U32> spirv = {result.cbegin(), result.cend()};

        s_Cached.insert({code, {stage, spirv}});

        return spirv;
    }
} // namespace Rose
