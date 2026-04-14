#pragma once

#include <Canis/Entity.hpp>

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace SuperPupUtilities
{
    class SimpleObjectPool : public Canis::ScriptableEntity
    {
    public:
        struct Pool
        {
            std::string code = "";
            Canis::SceneAssetHandle prefab = {};
            int size = 1;
        };

        static constexpr const char* ScriptName = "SuperPupUtilities::SimpleObjectPool";

        static SimpleObjectPool* GetInstance();

        std::vector<Pool> pools = {};
        bool logWarnings = true;

        explicit SimpleObjectPool(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;

        Canis::Entity* SpawnFromPool(
            const std::string& _code,
            const Canis::Vector3& _position,
            const Canis::Vector3& _rotation);

    private:
        static SimpleObjectPool* s_instance;

        std::unordered_map<std::string, std::queue<Canis::Entity*>> m_poolDictionary = {};
        bool m_initialized = false;

        void ApplySpawnTransform(
            Canis::Entity& _entity,
            const Canis::Vector3& _position,
            const Canis::Vector3& _rotation) const;
        void ForEachEntityInHierarchy(
            Canis::Entity& _root,
            const std::function<void(Canis::Entity&)>& _func) const;
        void ResetHierarchyPhysics(Canis::Entity& _root) const;
        void SetHierarchyActive(Canis::Entity& _root, bool _active) const;
        void ForceReadyHierarchy(Canis::Entity& _root) const;
        void BuildPools();
        Canis::Entity* CreatePooledEntity(const Pool& _pool);
    };

    void RegisterSimpleObjectPoolScript(Canis::App& _app);
    void UnRegisterSimpleObjectPoolScript(Canis::App& _app);
}
