#pragma once

#include <Canis/Entity.hpp>

namespace RollABall
{
    class PickupSpinner : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "RollABall::PickupSpinner";

        float spinSpeedDegrees = 120.0f;
        float flashMinFrequency = 1.2f;
        float flashMaxFrequency = 2.8f;
        float flashMinIntensity = 0.75f;
        float flashMaxIntensity = 1.35f;
        float flashPulseAmplitude = 0.45f;

        explicit PickupSpinner(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        
        void CheckSensorEnter();

    private:
        float m_flashPhase = 0.0f;
        float m_flashFrequency = 2.0f;
        float m_flashBaseIntensity = 1.0f;
    };

    void RegisterPickupSpinnerScript(Canis::App& _app);
    void UnRegisterPickupSpinnerScript(Canis::App& _app);
}
