#include <Player/PlacementController.hpp>

#include <Environment/VoxelTerrainChunk.hpp>
#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

namespace
{
    constexpr const char* kFurnaceItemName = "Furnace";
    constexpr const char* kCoiningPressItemName = "Coining Press";
    constexpr const char* kJumpPadItemName = "Jump Pad";
    constexpr const char* kRockItemName = "Rock";
}

ScriptConf placementControllerConf = {};

void RegisterPlacementControllerScript(App& _app)
{
    REGISTER_PROPERTY(placementControllerConf, PlacementController, furnacePrefab);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, coiningPressPrefab);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, boostPadPrefab);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, startingFurnaceCount);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, startingCoiningPressCount);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, startingBoostPadCount);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, placementRange);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, placementGroundMask);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, placementBlockMask);
    REGISTER_PROPERTY(placementControllerConf, PlacementController, minPlacementUpDot);

    DEFAULT_CONFIG_AND_REQUIRED(placementControllerConf, PlacementController, Transform, SuperPupUtilities::Inventory);

    placementControllerConf.DEFAULT_DRAW_INSPECTOR(PlacementController);

    _app.RegisterScript(placementControllerConf);
}

DEFAULT_UNREGISTER_SCRIPT(placementControllerConf, PlacementController)

void PlacementController::Create() {}

void PlacementController::Ready()
{
    m_cameraEntity = entity.scene.GetEntityWithTag("MainCamera");
    SeedStartingInventory();
}

void PlacementController::Destroy() {}

void PlacementController::Update(float _dt)
{
    (void)_dt;

    SuperPupUtilities::Inventory* inventory = entity.GetScript<SuperPupUtilities::Inventory>();
    if (inventory == nullptr)
        return;

    InputManager& input = entity.scene.GetInputManager();
    const int scroll = input.VerticalScroll();
    if (scroll > 0)
        inventory->SelectRelative(-1);
    else if (scroll < 0)
        inventory->SelectRelative(1);

    if (input.JustRightClicked())
        (void)TryPlaceSelectedItem();
}

void PlacementController::SeedStartingInventory()
{
    SuperPupUtilities::Inventory* inventory = entity.GetScript<SuperPupUtilities::Inventory>();
    if (inventory == nullptr)
        return;

    const int furnaceCount = inventory->GetCount(kFurnaceItemName);
    if (startingFurnaceCount > furnaceCount)
        inventory->Add(kFurnaceItemName, startingFurnaceCount - furnaceCount);

    const int coiningPressCount = inventory->GetCount(kCoiningPressItemName);
    if (startingCoiningPressCount > coiningPressCount)
        inventory->Add(kCoiningPressItemName, startingCoiningPressCount - coiningPressCount);

    const int jumpPadCount = inventory->GetCount(kJumpPadItemName);
    if (startingBoostPadCount > jumpPadCount)
        inventory->Add(kJumpPadItemName, startingBoostPadCount - jumpPadCount);
}

Canis::SceneAssetHandle PlacementController::GetSelectedPrefab(const std::string &_itemName) const
{
    if (_itemName == kFurnaceItemName)
        return furnacePrefab;
    if (_itemName == kCoiningPressItemName)
        return coiningPressPrefab;
    if (_itemName == kJumpPadItemName)
        return boostPadPrefab;

    return {};
}

bool PlacementController::TryPlaceTerrainBlock(SuperPupUtilities::Inventory &_inventory, const std::string &_itemName)
{
    if (m_cameraEntity == nullptr || !m_cameraEntity->HasComponent<Transform>())
        return false;

    Transform& cameraTransform = m_cameraEntity->GetComponent<Transform>();
    const Vector3 rayOrigin = cameraTransform.GetGlobalPosition();
    const Vector3 rayDirection = cameraTransform.GetForward();

    RaycastHit hit = {};
    if (!entity.scene.Raycast(rayOrigin, rayDirection, hit, placementRange, placementBlockMask))
        return false;

    VoxelTerrainChunk* sourceChunk = (hit.entity == nullptr) ? nullptr : hit.entity->GetScript<VoxelTerrainChunk>();
    if (sourceChunk == nullptr)
        return false;

    const Vector3 targetPoint = hit.point + (hit.normal * 0.05f);
    VoxelTerrainChunk* targetChunk = nullptr;
    glm::ivec3 targetCoords = glm::ivec3(0);

    for (Canis::Entity* candidate : entity.scene.GetEntities())
    {
        if (candidate == nullptr || !candidate->active)
            continue;

        VoxelTerrainChunk* chunk = candidate->GetScript<VoxelTerrainChunk>();
        if (chunk == nullptr)
            continue;

        glm::ivec3 blockCoords = glm::ivec3(0);
        if (!chunk->TryGetBlockCoordsFromWorldPoint(targetPoint, blockCoords))
            continue;

        targetChunk = chunk;
        targetCoords = blockCoords;
        break;
    }

    if (targetChunk == nullptr)
        return false;

    if (targetChunk->GetBlock(targetCoords.x, targetCoords.y, targetCoords.z) != TerrainBlockType::Air)
        return false;

    if (!_inventory.Remove(_itemName, 1))
        return false;

    targetChunk->SetBlock(targetCoords.x, targetCoords.y, targetCoords.z, TerrainBlockType::Rock);

    const bool rebuiltTarget = targetChunk->RebuildMesh();
    const bool rebuiltSource = (sourceChunk == targetChunk) ? true : sourceChunk->RebuildMesh();
    if (rebuiltTarget && rebuiltSource)
        return true;

    targetChunk->SetBlock(targetCoords.x, targetCoords.y, targetCoords.z, TerrainBlockType::Air);
    targetChunk->RebuildMesh();
    if (sourceChunk != targetChunk)
        sourceChunk->RebuildMesh();
    _inventory.Add(_itemName, 1);
    return false;
}

bool PlacementController::TryPlaceSelectedItem()
{
    if (m_cameraEntity == nullptr || !m_cameraEntity->HasComponent<Transform>())
        return false;

    SuperPupUtilities::Inventory* inventory = entity.GetScript<SuperPupUtilities::Inventory>();
    if (inventory == nullptr)
        return false;

    const int selectedSlot = inventory->GetSelectedSlotIndex();
    if (selectedSlot < 0)
        return false;

    const std::string itemName = inventory->GetSlotName(selectedSlot);
    if (itemName == kRockItemName)
        return TryPlaceTerrainBlock(*inventory, itemName);

    const Canis::SceneAssetHandle prefab = GetSelectedPrefab(itemName);
    if (prefab.path.empty())
        return false;

    Transform& cameraTransform = m_cameraEntity->GetComponent<Transform>();
    const Vector3 rayOrigin = cameraTransform.GetGlobalPosition();
    const Vector3 rayDirection = cameraTransform.GetForward();

    RaycastHit hit = {};
    bool foundSurface = entity.scene.Raycast(rayOrigin, rayDirection, hit, placementRange, placementBlockMask);
    if (!foundSurface)
        foundSurface = entity.scene.Raycast(rayOrigin, rayDirection, hit, placementRange, placementGroundMask);

    if (!foundSurface || hit.entity == nullptr || hit.normal.y < minPlacementUpDot)
        return false;

    if (!inventory->Remove(itemName, 1))
        return false;

    const Vector3 placementOffset = hit.point;
    const float playerYaw = entity.GetComponent<Transform>().rotation.y;

    for (Entity* root : entity.scene.Instantiate(prefab))
    {
        if (root != nullptr && root->HasComponent<Transform>())
        {
            Transform& rootTransform = root->GetComponent<Transform>();
            rootTransform.position += placementOffset;
            rootTransform.rotation.y = playerYaw;
        }
    }

    return true;
}
