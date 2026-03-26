#pragma once

#include <Canis/Entity.hpp>

#include <I_Interactable.hpp>

class CoiningPress : public Canis::ScriptableEntity, public I_Interactable
{
private:

public:
    static constexpr const char* ScriptName = "CoiningPress";

    int goldCount = 0;
    float timeLeft = 0.0f;
    float processingTime = 5.0f;
    Canis::SceneAssetHandle dropPrefab;

    CoiningPress(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);

    std::string GetMessage(const InteractionContext &_context) override;
    bool HandleInteraction(const InteractionContext &_context) override;
};

extern void RegisterCoiningPressScript(Canis::App& _app);
extern void UnRegisterCoiningPressScript(Canis::App& _app);
