#pragma once
#include "Rose/Base/Event.hpp"

namespace Rose {
    class Layer
    {
    public:
        virtual ~Layer() = default;

        virtual void OnTick(float dt) {}
        virtual void OnUpdate(float dt) {}
        virtual void OnRender() {}
    };

    class LayerStack final
    {
    public:
        LayerStack() { m_Layers.reserve(4); }

        ~LayerStack() { Clear(); }

        void PushLayer(Layer* layer)
        {
            m_Layers.emplace(m_Layers.begin() + m_LayerIndex, layer);
            m_LayerIndex++;
        }

        void PopLayer(Layer* layer)
        {
            if (auto it = std::ranges::find(m_Layers, layer); it != m_Layers.end())
            {
                delete layer;
                m_Layers.erase(it);
                m_LayerIndex--;
            }
        }

        void PushOverlay(Layer* overlay) { m_Layers.push_back(overlay); }

        void PopOverlay(Layer* overlay)
        {

            if (auto it = std::ranges::find(m_Layers, overlay); it != m_Layers.end())
            {
                delete overlay;
                m_Layers.erase(it);
            }
        }

        void Clear()
        {
            for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
                delete (*it);

            m_Layers.clear();
        }

        void Tick(float dt) const;
        void Update(float dt) const;
        void Render() const;

        void SetEventBus(EventBus* ebus) { m_EventBus = ebus; }

    private:
        std::vector<Layer*> m_Layers;
        EventBus* m_EventBus = nullptr;

        uint16_t m_LayerIndex = 0;
    };
} // namespace Rose
