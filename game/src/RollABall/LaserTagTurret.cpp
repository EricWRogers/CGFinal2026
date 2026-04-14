#include <RollABall/LaserTagTurret.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <SuperPupUtilities/Bullet.hpp>
#include <SuperPupUtilities/SimpleObjectPool.hpp>

#include <algorithm>
#include <cmath>

namespace RollABall
{
    namespace
    {
        constexpr float Tau = 6.28318530718f;
    }

    Canis::ScriptConf laserTagTurretConf = {};

    void RegisterLaserTagTurretScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, targetTag);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, poolCode);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, fireInterval);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, turnSpeedDegrees);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, fireAngleThresholdDegrees);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, muzzleOffset);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, projectileSpeed);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, projectileLifeTime);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, projectileHitImpulse);
        REGISTER_PROPERTY(laserTagTurretConf, RollABall::LaserTagTurret, logShots);

        DEFAULT_CONFIG_AND_REQUIRED(laserTagTurretConf, RollABall::LaserTagTurret, Canis::Transform);

        laserTagTurretConf.DEFAULT_DRAW_INSPECTOR(RollABall::LaserTagTurret);

        _app.RegisterScript(laserTagTurretConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(laserTagTurretConf, LaserTagTurret)

    void LaserTagTurret::Create()
    {
        entity.GetComponent<Canis::Transform>();
    }

    void LaserTagTurret::Ready()
    {
        m_target = FindTarget();
        m_fireCooldown = fireInterval;
    }

    void LaserTagTurret::Destroy() {}

    void LaserTagTurret::Update(float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>())
            return;

        if (m_target == nullptr || !m_target->active || m_target->tag != targetTag)
            m_target = FindTarget();

        if (m_target == nullptr || !m_target->HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 targetPosition = m_target->GetComponent<Canis::Transform>().GetGlobalPosition();
        Canis::Vector3 toTarget = targetPosition - transform.GetGlobalPosition();
        toTarget.y = 0.0f;

        if (glm::length(toTarget) <= 0.001f)
            return;

        const float angleError = RotateTowards(transform, toTarget, _dt);
        if (m_fireCooldown > 0.0f)
            m_fireCooldown -= _dt;

        if (m_fireCooldown > 0.0f)
            return;

        const float fireAngleThreshold = fireAngleThresholdDegrees * Canis::DEG2RAD;
        if (std::abs(angleError) > fireAngleThreshold)
            return;

        Fire(GetMuzzlePosition(transform), toTarget);
        m_fireCooldown = fireInterval;
    }

    Canis::Entity* LaserTagTurret::FindTarget() const
    {
        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag(targetTag))
        {
            if (candidate != nullptr && candidate->active)
                return candidate;
        }

        return nullptr;
    }

    Canis::Vector3 LaserTagTurret::GetMuzzlePosition(const Canis::Transform& _transform) const
    {
        return _transform.GetGlobalPosition()
            + (_transform.GetRight() * muzzleOffset.x)
            + (_transform.GetUp() * muzzleOffset.y)
            + (_transform.GetForward() * muzzleOffset.z);
    }

    float LaserTagTurret::RotateTowards(Canis::Transform& _transform, const Canis::Vector3& _direction, float _dt) const
    {
        const Canis::Vector3 flatDirection = glm::normalize(Canis::Vector3(_direction.x, 0.0f, _direction.z));
        const float targetYaw = std::atan2(-flatDirection.x, -flatDirection.z);
        const float yawError = std::remainder(targetYaw - _transform.rotation.y, Tau);
        const float maxStep = turnSpeedDegrees * Canis::DEG2RAD * _dt;
        const float appliedStep = std::clamp(yawError, -maxStep, maxStep);

        _transform.rotation.y += appliedStep;
        return std::remainder(targetYaw - _transform.rotation.y, Tau);
    }

    void LaserTagTurret::Fire(const Canis::Vector3& _position, const Canis::Vector3& _direction)
    {
        SuperPupUtilities::SimpleObjectPool* pool = SuperPupUtilities::SimpleObjectPool::GetInstance();
        if (pool == nullptr)
        {
            if (logShots)
                Canis::Debug::Warning("LaserTagTurret: no SimpleObjectPool instance was found.");
            return;
        }

        const Canis::Vector3 flatDirection = glm::normalize(Canis::Vector3(_direction.x, 0.0f, _direction.z));
        const float yaw = std::atan2(-flatDirection.x, -flatDirection.z);
        const Canis::Vector3 rotation = Canis::Vector3(0.0f, yaw, 0.0f);

        Canis::Entity* projectile = pool->SpawnFromPool(poolCode, _position, rotation);
        if (projectile == nullptr)
            return;

        if (SuperPupUtilities::Bullet* bullet = projectile->GetScript<SuperPupUtilities::Bullet>())
        {
            bullet->speed = projectileSpeed;
            bullet->lifeTime = projectileLifeTime;
            bullet->hitImpulse = projectileHitImpulse;
            bullet->Launch();
        }

        if (logShots)
            Canis::Debug::Log("%s fired from pool '%s'.", entity.name.c_str(), poolCode.c_str());
    }
}
