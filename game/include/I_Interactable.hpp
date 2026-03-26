#pragma once
#include <string>

namespace Canis
{
    class Entity;
    struct RaycastHit;
}

struct InteractionContext
{
    Canis::Entity* interactingEntity = nullptr;
    const Canis::RaycastHit* hit = nullptr;
};

class I_Interactable
{
public:
    virtual ~I_Interactable() = default;
    virtual std::string GetMessage(const InteractionContext &_context) = 0;
    virtual bool HandleInteraction(const InteractionContext &_context) = 0; // return true if interacted with
};
