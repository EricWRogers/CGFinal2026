#include <Environment/GenerateTerrain.hpp>

#include <Environment/VoxelTerrainChunk.hpp>

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>

#include <algorithm>
#include <cmath>

namespace
{
    unsigned int HashU32(unsigned int _value)
    {
        _value ^= _value >> 16u;
        _value *= 0x7feb352du;
        _value ^= _value >> 15u;
        _value *= 0x846ca68bu;
        _value ^= _value >> 16u;
        return _value;
    }

    float Hash01(int _x, int _y, int _z, int _seed)
    {
        unsigned int value = static_cast<unsigned int>(_seed);
        value ^= HashU32(static_cast<unsigned int>(_x) * 0x1f123bb5u);
        value ^= HashU32(static_cast<unsigned int>(_y) * 0x9e3779b9u);
        value ^= HashU32(static_cast<unsigned int>(_z) * 0x94d049bbu);
        return static_cast<float>(HashU32(value) & 0x00ffffffu) / static_cast<float>(0x01000000u);
    }

    float SmoothStep(float _t)
    {
        return _t * _t * (3.0f - (2.0f * _t));
    }

    float Lerp(float _a, float _b, float _t)
    {
        return _a + ((_b - _a) * _t);
    }

    float ValueNoise2D(float _x, float _z, int _seed)
    {
        const int x0 = static_cast<int>(std::floor(_x));
        const int z0 = static_cast<int>(std::floor(_z));
        const int x1 = x0 + 1;
        const int z1 = z0 + 1;

        const float tx = SmoothStep(_x - static_cast<float>(x0));
        const float tz = SmoothStep(_z - static_cast<float>(z0));

        const float v00 = Hash01(x0, 0, z0, _seed);
        const float v10 = Hash01(x1, 0, z0, _seed);
        const float v01 = Hash01(x0, 0, z1, _seed);
        const float v11 = Hash01(x1, 0, z1, _seed);

        return Lerp(Lerp(v00, v10, tx), Lerp(v01, v11, tx), tz);
    }

    float ValueNoise3D(float _x, float _y, float _z, int _seed)
    {
        const int x0 = static_cast<int>(std::floor(_x));
        const int y0 = static_cast<int>(std::floor(_y));
        const int z0 = static_cast<int>(std::floor(_z));
        const int x1 = x0 + 1;
        const int y1 = y0 + 1;
        const int z1 = z0 + 1;

        const float tx = SmoothStep(_x - static_cast<float>(x0));
        const float ty = SmoothStep(_y - static_cast<float>(y0));
        const float tz = SmoothStep(_z - static_cast<float>(z0));

        const float v000 = Hash01(x0, y0, z0, _seed);
        const float v100 = Hash01(x1, y0, z0, _seed);
        const float v010 = Hash01(x0, y1, z0, _seed);
        const float v110 = Hash01(x1, y1, z0, _seed);
        const float v001 = Hash01(x0, y0, z1, _seed);
        const float v101 = Hash01(x1, y0, z1, _seed);
        const float v011 = Hash01(x0, y1, z1, _seed);
        const float v111 = Hash01(x1, y1, z1, _seed);

        const float ix00 = Lerp(v000, v100, tx);
        const float ix10 = Lerp(v010, v110, tx);
        const float ix01 = Lerp(v001, v101, tx);
        const float ix11 = Lerp(v011, v111, tx);

        return Lerp(Lerp(ix00, ix10, ty), Lerp(ix01, ix11, ty), tz);
    }

    float FractalNoise2D(float _x, float _z, int _seed, int _octaves)
    {
        float total = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float normalization = 0.0f;

        for (int octave = 0; octave < _octaves; ++octave)
        {
            total += ValueNoise2D(_x * frequency, _z * frequency, _seed + (octave * 31)) * amplitude;
            normalization += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return normalization > 0.0f ? (total / normalization) : 0.0f;
    }

    float FractalNoise3D(float _x, float _y, float _z, int _seed, int _octaves)
    {
        float total = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float normalization = 0.0f;

        for (int octave = 0; octave < _octaves; ++octave)
        {
            total += ValueNoise3D(_x * frequency, _y * frequency, _z * frequency, _seed + (octave * 57)) * amplitude;
            normalization += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return normalization > 0.0f ? (total / normalization) : 0.0f;
    }

    int GetTerrainHeight(const GenerateTerrain &_terrain, int _globalX, int _globalZ)
    {
        const float broad = FractalNoise2D(
            static_cast<float>(_globalX) * _terrain.heightNoiseScale,
            static_cast<float>(_globalZ) * _terrain.heightNoiseScale,
            _terrain.seed,
            4);
        const float detail = FractalNoise2D(
            static_cast<float>(_globalX) * _terrain.detailNoiseScale,
            static_cast<float>(_globalZ) * _terrain.detailNoiseScale,
            _terrain.seed + 101,
            2);
        const float blended = std::clamp((broad * 0.75f) + (detail * 0.25f), 0.0f, 1.0f);
        return _terrain.baseHeight + static_cast<int>(std::round(blended * static_cast<float>(_terrain.maxHeightVariation)));
    }

    TerrainBlockType GetTerrainBlockType(const GenerateTerrain &_terrain, int _globalX, int _y, int _globalZ, int _columnHeight)
    {
        const bool isSurface = (_y == (_columnHeight - 1));
        if (!isSurface && _y > 2 && _y < (_columnHeight - 2))
        {
            const float caveNoise = FractalNoise3D(
                static_cast<float>(_globalX) * _terrain.caveNoiseScale,
                static_cast<float>(_y) * _terrain.caveNoiseScale,
                static_cast<float>(_globalZ) * _terrain.caveNoiseScale,
                _terrain.seed + 401,
                3);
            if (caveNoise > 0.72f)
                return TerrainBlockType::Air;
        }

        if (isSurface && _columnHeight >= _terrain.surfaceIceHeight)
            return TerrainBlockType::Ice;

        const float uraniumNoise = FractalNoise3D(
            static_cast<float>(_globalX) * 0.18f,
            static_cast<float>(_y) * 0.18f,
            static_cast<float>(_globalZ) * 0.18f,
            _terrain.seed + 601,
            2);
        if (_y < (_columnHeight / 2) && uraniumNoise > 0.80f)
            return TerrainBlockType::Uranium;

        const float goldNoise = FractalNoise3D(
            static_cast<float>(_globalX) * 0.14f,
            static_cast<float>(_y) * 0.14f,
            static_cast<float>(_globalZ) * 0.14f,
            _terrain.seed + 777,
            2);
        if (_y < (_columnHeight - 1) && goldNoise > 0.78f)
            return TerrainBlockType::Gold;

        return TerrainBlockType::Rock;
    }
}

ScriptConf generateTerrainConf = {};

void RegisterGenerateTerrainScript(Canis::App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(generateTerrainConf, GenerateTerrain, Transform);

    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, seed);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, chunksX);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, chunksZ);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, chunkSize);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, chunkHeight);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, baseHeight);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, maxHeightVariation);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, surfaceIceHeight);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, heightNoiseScale);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, detailNoiseScale);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, caveNoiseScale);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, rockDropPrefab);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, iceDropPrefab);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, goldDropPrefab);
    REGISTER_PROPERTY(generateTerrainConf, GenerateTerrain, uraniumDropPrefab);

    generateTerrainConf.DEFAULT_DRAW_INSPECTOR(GenerateTerrain);

    _app.RegisterScript(generateTerrainConf);
}

DEFAULT_UNREGISTER_SCRIPT(generateTerrainConf, GenerateTerrain)

void GenerateTerrain::Create() {}

void GenerateTerrain::Ready()
{
    if (m_generated || !entity.HasComponent<Canis::Transform>())
        return;

    chunksX = std::max(1, chunksX);
    chunksZ = std::max(1, chunksZ);
    chunkSize = std::max(4, chunkSize);
    chunkHeight = std::max(4, chunkHeight);
    baseHeight = std::clamp(baseHeight, 1, chunkHeight - 1);
    maxHeightVariation = std::max(1, maxHeightVariation);
    surfaceIceHeight = std::max(1, surfaceIceHeight);

    const int rockMaterialId = Canis::AssetManager::LoadMaterial("assets/materials/terrain_rock.material");
    const int iceMaterialId = Canis::AssetManager::LoadMaterial("assets/materials/terrain_ice.material");
    const int goldMaterialId = Canis::AssetManager::LoadMaterial("assets/materials/terrain_gold.material");
    const int uraniumMaterialId = Canis::AssetManager::LoadMaterial("assets/materials/terrain_uranium.material");

    for (int chunkZIndex = 0; chunkZIndex < chunksZ; ++chunkZIndex)
    {
        for (int chunkXIndex = 0; chunkXIndex < chunksX; ++chunkXIndex)
        {
            Canis::Entity *chunkEntity = entity.scene.CreateEntity("TerrainChunk");
            if (chunkEntity == nullptr)
                continue;

            Canis::Transform &chunkTransform = chunkEntity->GetComponent<Canis::Transform>();
            chunkTransform.SetParent(&entity);
            chunkTransform.position = Canis::Vector3(
                static_cast<float>(chunkXIndex * chunkSize),
                0.0f,
                static_cast<float>(chunkZIndex * chunkSize));

            Canis::Rigidbody &rigidbody = chunkEntity->GetComponent<Canis::Rigidbody>();
            rigidbody.motionType = Canis::RigidbodyMotionType::STATIC;
            rigidbody.useGravity = false;
            rigidbody.layer = 5u;

            Canis::MeshCollider &meshCollider = chunkEntity->GetComponent<Canis::MeshCollider>();
            meshCollider.active = true;
            meshCollider.useAttachedModel = true;

            chunkEntity->GetComponent<Canis::Model>();
            chunkEntity->GetComponent<Canis::Material>();

            VoxelTerrainChunk *chunk = chunkEntity->AddScript<VoxelTerrainChunk>();
            chunk->sizeX = chunkSize;
            chunk->sizeY = chunkHeight;
            chunk->sizeZ = chunkSize;
            chunk->rockMaterialId = rockMaterialId;
            chunk->iceMaterialId = iceMaterialId;
            chunk->goldMaterialId = goldMaterialId;
            chunk->uraniumMaterialId = uraniumMaterialId;
            chunk->rockDropPrefab = rockDropPrefab;
            chunk->iceDropPrefab = iceDropPrefab;
            chunk->goldDropPrefab = goldDropPrefab;
            chunk->uraniumDropPrefab = uraniumDropPrefab;
            chunk->Resize(chunkSize, chunkHeight, chunkSize);

            for (int localZ = 0; localZ < chunkSize; ++localZ)
            {
                for (int localX = 0; localX < chunkSize; ++localX)
                {
                    const int globalX = (chunkXIndex * chunkSize) + localX;
                    const int globalZ = (chunkZIndex * chunkSize) + localZ;
                    const int columnHeight = std::min(chunkHeight, GetTerrainHeight(*this, globalX, globalZ));

                    for (int y = 0; y < columnHeight; ++y)
                    {
                        const TerrainBlockType blockType = GetTerrainBlockType(*this, globalX, y, globalZ, columnHeight);
                        if (blockType != TerrainBlockType::Air)
                            chunk->SetBlock(localX, y, localZ, blockType);
                    }
                }
            }

            chunk->RebuildMesh();
        }
    }

    m_generated = true;
}

void GenerateTerrain::Destroy() {}

void GenerateTerrain::Update(float _dt)
{
    (void)_dt;
}
