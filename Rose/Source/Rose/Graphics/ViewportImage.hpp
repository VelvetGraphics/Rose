#pragma once

#include <imgui.h>

namespace Rose {
    class ViewportImage : public RefCounted
    {
    public:
        ~ViewportImage() override = default;

        static Ref<ViewportImage> Create();

        virtual void UpdateTexture() = 0;
        virtual ImTextureID GetTextureID() const = 0;

        virtual void Resize(U32 width, U32 height) = 0;
        virtual ImVec2 Size() = 0;

    protected:
        ViewportImage() = default;
    };
} // namespace Rose
