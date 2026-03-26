#include <Items/GoldCoin.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/ConfigHelper.hpp>

ScriptConf goldCoinConf = {};

void RegisterGoldCoinScript(App& _app)
{
    DEFAULT_CONFIG_AND_REQUIRED(goldCoinConf, GoldCoin, RectTransform);

    goldCoinConf.DEFAULT_DRAW_INSPECTOR(GoldCoin);

    _app.RegisterScript(goldCoinConf);
}

DEFAULT_UNREGISTER_SCRIPT(goldCoinConf, GoldCoin)

void GoldCoin::Create() {}

void GoldCoin::Ready() {}

void GoldCoin::Destroy() {}

void GoldCoin::Update(float _dt) {}

std::string GoldCoin::GetName()
{
    return "Gold Coin";
}

std::string GoldCoin::GetMessage(const InteractionContext &_context)
{
    (void)_context;
    return std::string("Press E to Pickup ") + GetName();
}

bool GoldCoin::HandleInteraction(const InteractionContext &_context)
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
