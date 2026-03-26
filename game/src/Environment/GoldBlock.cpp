#include <Environment/GoldBlock.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf goldBlockConf = {};

void RegisterGoldBlockScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(goldBlockConf, GoldBlock);
    REGISTER_PROPERTY(goldBlockConf, GoldBlock, dropPrefab);

    goldBlockConf.DEFAULT_DRAW_INSPECTOR(GoldBlock);

    _app.RegisterScript(goldBlockConf);
}

DEFAULT_UNREGISTER_SCRIPT(goldBlockConf, GoldBlock)

void GoldBlock::Create() {}

void GoldBlock::Ready() {}

void GoldBlock::Destroy() {}

void GoldBlock::Update(float _dt) {}

std::string GoldBlock::GetName()
{
    return "Gold Block";
}

std::string GoldBlock::GetMessage(const InteractionContext &_context)
{
    (void)_context;
    return std::string("Left Click to Break ") + GetName();
}

bool GoldBlock::HandleInteraction(const InteractionContext &_context)
{
    (void)_context;
    InputManager& input = entity.scene.GetInputManager();
    if (!input.LeftClickReleased())
        return false;

    const Vector3 spawnOffset = entity.HasComponent<Transform>()
        ? entity.GetComponent<Transform>().GetGlobalPosition()
        : Vector3(0.0f);

    entity.Destroy();

    for (int i = 0; i < 5; i++)
    {
        Vector3 randomOffset;
        randomOffset.x = ((rand() % 100) * 0.01) - 0.5;
        randomOffset.y = ((rand() % 100) * 0.01) - 0.5;
        randomOffset.z = ((rand() % 100) * 0.01) - 0.5;

        for (Entity *root : entity.scene.Instantiate(dropPrefab))
        {
            if (root != nullptr && root->HasComponent<Transform>())
                root->GetComponent<Transform>().position += spawnOffset + randomOffset;
        }
    }

    return true;
}
