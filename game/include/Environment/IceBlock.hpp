#pragma once

#include <Environment/I_Block.hpp>
#include <I_Interactable.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/Entity.hpp>

class IceBlock : public Canis::ScriptableEntity, public I_Block, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "IceBlock";

    explicit IceBlock(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    Canis::SceneAssetHandle dropPrefab = {};

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetName() override;
    std::string GetMessage(const InteractionContext &_context) override;
    bool HandleInteraction(const InteractionContext &_context) override;
};

extern void RegisterIceBlockScript(Canis::App& _app);
extern void UnRegisterIceBlockScript(Canis::App& _app);
