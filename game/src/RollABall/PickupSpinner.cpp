#include <RollABall/PickupSpinner.hpp>

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <RollABall/PlayerController.hpp>

#include <algorithm>
#include <cmath>
#include <random>

namespace RollABall
{
    ScriptConf pickUpConf = {};

    void RegisterPickupSpinnerScript(App& _app)
    {
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, spinSpeedDegrees);
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, flashMinFrequency);
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, flashMaxFrequency);
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, flashMinIntensity);
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, flashMaxIntensity);
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, flashPulseAmplitude);

        DEFAULT_CONFIG_AND_REQUIRED(pickUpConf, RollABall::PickupSpinner, Transform, Material);

        pickUpConf.DEFAULT_DRAW_INSPECTOR(RollABall::PickupSpinner);

        _app.RegisterScript(pickUpConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(pickUpConf, PickupSpinner)

    void PickupSpinner::Create()
    {
        entity.GetComponent<Transform>();

        Rigidbody& rigidbody = entity.GetComponent<Rigidbody>();
        rigidbody.motionType = RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Vector3(0.0f);
        rigidbody.angularVelocity = Vector3(0.0f);

        if (!entity.HasComponent<BoxCollider>()
            && !entity.HasComponent<SphereCollider>()
            && !entity.HasComponent<CapsuleCollider>())
        {
            entity.GetComponent<BoxCollider>();
        }

        const float minFrequency = std::min(flashMinFrequency, flashMaxFrequency);
        const float maxFrequency = std::max(flashMinFrequency, flashMaxFrequency);
        const float minIntensity = std::min(flashMinIntensity, flashMaxIntensity);
        const float maxIntensity = std::max(flashMinIntensity, flashMaxIntensity);

        const u64 uuidValue = static_cast<u64>(entity.uuid);
        u32 seed = static_cast<u32>(uuidValue ^ (uuidValue >> 32u));
        if (seed == 0u)
            seed = static_cast<u32>(std::random_device{}());

        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> frequencyDistribution(minFrequency, maxFrequency);
        std::uniform_real_distribution<float> intensityDistribution(minIntensity, maxIntensity);
        std::uniform_real_distribution<float> phaseDistribution(0.0f, TAU);
        m_flashFrequency = frequencyDistribution(rng);
        m_flashBaseIntensity = intensityDistribution(rng);
        m_flashPhase = phaseDistribution(rng);

        if (entity.HasComponent<Material>())
            entity.GetComponent<Material>().materialFields.SetFloat("pickupFlash", m_flashBaseIntensity);
    }

    void PickupSpinner::Ready() {}

    void PickupSpinner::Destroy() {}

    void PickupSpinner::Update(float _dt)
    {
        if (!entity.HasComponent<Transform>())
            return;

        Transform& transform = entity.GetComponent<Transform>();
        transform.rotation.y += spinSpeedDegrees * DEG2RAD * _dt;

        m_flashPhase += _dt * m_flashFrequency * TAU;
        if (m_flashPhase >= TAU)
            m_flashPhase = std::fmod(m_flashPhase, TAU);

        const float pulseAmplitude = std::max(0.0f, flashPulseAmplitude);
        const float flash = std::max(0.0f, m_flashBaseIntensity + (std::sin(m_flashPhase) * pulseAmplitude));
        if (entity.HasComponent<Material>())
            entity.GetComponent<Material>().materialFields.SetFloat("pickupFlash", flash);

        CheckSensorEnter();
    }

    void PickupSpinner::CheckSensorEnter()
    {
        if (!entity.HasComponents<BoxCollider,Rigidbody>())
            return;

        Entity* collectingPlayer = nullptr;

        for (Entity* other : entity.GetComponent<BoxCollider>().entered)
        {
            if (!other->active)
                continue;

            if (other->HasScript<RollABall::PlayerController>()) {
                collectingPlayer = other;
                break;
            }
        }

        if (collectingPlayer == nullptr)
            return;

        if (PlayerController* playerController = collectingPlayer->GetScript<PlayerController>())
        {
            playerController->CollectPickup();
            entity.Destroy();
        }
    }
}
