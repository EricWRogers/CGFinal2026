#pragma once

#include <Environment/I_Block.hpp>
#include <I_Interactable.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/Entity.hpp>

class UraniumBlock : public Canis::ScriptableEntity, public I_Block, public I_Interactable
{
public:
    static constexpr const char* ScriptName = "UraniumBlock";

    explicit UraniumBlock(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    Canis::SceneAssetHandle dropPrefab = {};

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetName() override;
    std::string GetMessage(const InteractionContext &_context) override;
    bool HandleInteraction(const InteractionContext &_context) override;
};

extern void RegisterUraniumBlockScript(Canis::App& _app);
extern void UnRegisterUraniumBlockScript(Canis::App& _app);
