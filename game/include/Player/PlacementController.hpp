#pragma once

#include <Canis/Entity.hpp>
#include <Canis/AssetHandle.hpp>

namespace SuperPupUtilities
{
    class Inventory;
}

class PlacementController : public Canis::ScriptableEntity
{
private:
    Canis::Entity* m_cameraEntity = nullptr;

    void SeedStartingInventory();
    Canis::SceneAssetHandle GetSelectedPrefab(const std::string &_itemName) const;
    bool TryPlaceTerrainBlock(SuperPupUtilities::Inventory &_inventory, const std::string &_itemName);
    bool TryPlaceSelectedItem();

public:
    static constexpr const char* ScriptName = "PlacementController";

    Canis::SceneAssetHandle furnacePrefab = {"assets/prefabs/furnace.scene"};
    Canis::SceneAssetHandle coiningPressPrefab = {"assets/prefabs/coining_press.scene"};
    Canis::SceneAssetHandle boostPadPrefab = {"assets/prefabs/boost_pad.scene"};
    int startingFurnaceCount = 1;
    int startingCoiningPressCount = 1;
    int startingBoostPadCount = 5;
    float placementRange = 12.0f;
    Canis::Mask placementGroundMask = Canis::Mask(1u);
    Canis::Mask placementBlockMask = Canis::Mask(4u);
    float minPlacementUpDot = 0.35f;

    explicit PlacementController(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create();
    void Ready();
    void Destroy();
    void Update(float _dt);
};

extern void RegisterPlacementControllerScript(Canis::App& _app);
extern void UnRegisterPlacementControllerScript(Canis::App& _app);
