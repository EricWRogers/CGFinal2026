#include <Items/GoldOre.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/ConfigHelper.hpp>

ScriptConf goldOreConf = {};

void RegisterGoldOreScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(goldOreConf, GoldOre, RectTransform);

    goldOreConf.DEFAULT_DRAW_INSPECTOR(GoldOre);

    _app.RegisterScript(goldOreConf);
}

DEFAULT_UNREGISTER_SCRIPT(goldOreConf, GoldOre)

void GoldOre::Create() {}

void GoldOre::Ready() {}

void GoldOre::Destroy() {}

void GoldOre::Update(float _dt) {}

std::string GoldOre::GetName()
{
    return "Gold Ore";
}

std::string GoldOre::GetMessage(const InteractionContext &_context)
{
    (void)_context;
    return std::string("Press E to Pickup ") + GetName();
}

bool GoldOre::HandleInteraction(const InteractionContext &_context)
{
    InputManager& input = entity.scene.GetInputManager();

    if (input.GetKey(Key::E))
    {
        if (_context.interactingEntity != nullptr)
            if (SuperPupUtilities::Inventory* inventory = _context.interactingEntity->GetScript<SuperPupUtilities::Inventory>())
                inventory->Add(*this, 1);

        entity.Destroy();
        return true;
    }

    return false;
}
