#pragma once

namespace Rose {
    class Mesh : public RefCounted
    {
    public:
        ~Mesh() override = default;

        static Ref<Mesh> Create(std::string path);

        virtual void Reload() = 0;
        virtual void Bind() const = 0;
        virtual void Draw() const = 0;

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    protected:
        Mesh(std::string&& path) : m_Path(std::move(path)) {}

    protected:
        std::string m_Path;
    };
} // namespace Rose
