#include "Shader.hpp"

#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"
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
        }

        ShaderStage ShaderStageFromString(const std::string& string)
        {
            if (string == std::string("Vertex"))
                return ShaderStage::Vertex;

            if (string == std::string("Fragment"))
                return ShaderStage::Fragment;

            ASSERT(false, "Invalid string for shader stage");
        }

        vk::ShaderModule CreateShaderModule(const std::vector<U32>& bytecode)
        {
            vk::ShaderModuleCreateInfo createInfo = {};
            createInfo.codeSize = bytecode.size() * sizeof(U32);
            createInfo.pCode = bytecode.data();

            auto shaderModuleResult = GraphicsAPI::Device().createShaderModule(createInfo);
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
            }

            return std::move(vulkanAttributes);
        }
    } // namespace

    Shader::~Shader()
    {
        GraphicsAPI::Device().destroyPipelineLayout(m_Layout);
        GraphicsAPI::Device().destroyPipeline(m_Pipeline);
    }

    void Shader::Bind() const
    {
        GraphicsAPI::Submit([this](vk::CommandBuffer cmdBuffer) {
            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
        });
    }

    void Shader::Reload()
    {
        std::vector<U32> vertShaderBytecode, fragShaderBytecode;
        {
            std::ifstream file(m_Path, std::ios::ate);
            ASSERT(file.is_open(), std::string("Failed to open file at path: " + m_Path));

            S64 fileSize = file.tellg();
            file.seekg(0);

            std::string code;
            code.resize(fileSize);

            file.read(code.data(), fileSize);
            file.close();

            ShaderSources sources = PreProcess(code);
            vertShaderBytecode = Compile(m_Path, ShaderStage::Vertex, sources.VertexShader);
            fragShaderBytecode = Compile(m_Path, ShaderStage::Fragment, sources.FragmentShader);
        }

        CreateGraphicsPipeline(std::move(vertShaderBytecode), std::move(fragShaderBytecode));
    }

    ShaderSources Shader::PreProcess(const std::string& code)
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
                case ShaderStage::Fragment:
                    result.FragmentShader = (pos == std::string::npos) ? code.substr(nextLinePos)
                                                                       : code.substr(nextLinePos, pos - nextLinePos);
            }
        }

        return result;
    }

    void Shader::CreateGraphicsPipeline(std::vector<U32>&& vertShaderBytecode, std::vector<U32>&& fragShaderBytecode)
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

        vk::Extent2D swapChainExtent = GraphicsAPI::SwapChainExtent();
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
        rasterizer.frontFace = vk::FrontFace::eClockwise;
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
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pushConstantRangeCount = 0;

        auto layoutResult = GraphicsAPI::Device().createPipelineLayout(layoutInfo);
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

        vk::Format colorAttachmentFormat = GraphicsAPI::SwapChainSurfaceFormat();

        auto& pipelineRendering = pipelineCreateInfoChain.get<vk::PipelineRenderingCreateInfo>();
        pipelineRendering.colorAttachmentCount = 1;
        pipelineRendering.pColorAttachmentFormats = &colorAttachmentFormat;
        pipelineRendering.depthAttachmentFormat = GraphicsAPI::DepthFormat();

        auto pipelineResult = GraphicsAPI::Device().createGraphicsPipeline(nullptr, pipelineInfo);
        ASSERT(pipelineResult.result == vk::Result::eSuccess, "Failed to create graphics pipeline");
        m_Pipeline = pipelineResult.value;

        GraphicsAPI::Device().destroyShaderModule(vertShaderModule);
        GraphicsAPI::Device().destroyShaderModule(fragShaderModule);
    }

    std::vector<U32> Shader::Compile(const std::string& path, ShaderStage stage, const std::string& code)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc::CompilationResult result =
                compiler.CompileGlslToSpv(code.c_str(), TranslateShaderStage(stage), path.c_str(), options);

        ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success,
               std::string("Failed to compile shader to assembly, message: " + result.GetErrorMessage()));

        return {result.cbegin(), result.cend()};
    }

    Ref<Shader> Shader::Create(std::string path) { return Ref<Shader>::Create(std::move(path)); }

    Shader::Shader(std::string&& path) : m_Path(std::move(path)) { Reload(); }
} // namespace Rose
