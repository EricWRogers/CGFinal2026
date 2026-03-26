#pragma once

#include <Canis/Entity.hpp>

class GenerateTerrain : public Canis::ScriptableEntity
{
private:
    bool m_generated = false;

public:
    static constexpr const char* ScriptName = "GenerateTerrain";

    int seed = 1337;
    int chunksX = 4;
    int chunksZ = 4;
    int chunkSize = 16;
    int chunkHeight = 24;
    int baseHeight = 6;
    int maxHeightVariation = 10;
    int surfaceIceHeight = 11;
    float heightNoiseScale = 0.075f;
    float detailNoiseScale = 0.16f;
    float caveNoiseScale = 0.12f;

    Canis::SceneAssetHandle rockDropPrefab = {};
    Canis::SceneAssetHandle iceDropPrefab = {};
    Canis::SceneAssetHandle goldDropPrefab = {};
    Canis::SceneAssetHandle uraniumDropPrefab = {};

    GenerateTerrain(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);
};

extern void RegisterGenerateTerrainScript(Canis::App& _app);
extern void UnRegisterGenerateTerrainScript(Canis::App& _app);
