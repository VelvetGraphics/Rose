#include "Layer.hpp"

#include "Rose/Base/EventTypes.hpp"
#include "Rose/Main/Main.hpp"

namespace Rose {

    void LayerStack::Tick(float dt) const
    {
        for (Layer* layer : m_Layers)
            layer->OnTick(dt);

        m_EventBus->Queue(std::make_unique<AppTickedEvent>());
    }

    void LayerStack::Update(float dt) const
    {
        for (Layer* layer : m_Layers)
            layer->OnUpdate(dt);

        m_EventBus->Queue(std::make_unique<AppUpdatedEvent>());
    }

    void LayerStack::Render() const
    {
        for (Layer* layer : m_Layers)
            layer->OnRender();

        m_EventBus->Queue(std::make_unique<AppRenderedEvent>());
    }
} // namespace Rose
