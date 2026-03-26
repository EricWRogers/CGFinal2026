#include <Machines/CoiningPress.hpp>

#include <SuperPupUtilities/Inventory.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/InputManager.hpp>

ScriptConf coiningPressConf = {};

void RegisterCoiningPressScript(App& _app)
{
    DEFAULT_CONFIG(coiningPressConf, CoiningPress);
    REGISTER_PROPERTY(coiningPressConf, CoiningPress, goldCount);
    REGISTER_PROPERTY(coiningPressConf, CoiningPress, timeLeft);
    REGISTER_PROPERTY(coiningPressConf, CoiningPress, processingTime);
    REGISTER_PROPERTY(coiningPressConf, CoiningPress, dropPrefab);

    coiningPressConf.DEFAULT_DRAW_INSPECTOR(CoiningPress);

    _app.RegisterScript(coiningPressConf);
}

DEFAULT_UNREGISTER_SCRIPT(coiningPressConf, CoiningPress)

void CoiningPress::Create() {}

void CoiningPress::Ready() {}

void CoiningPress::Destroy() {}

void CoiningPress::Update(float _dt) {
    if (timeLeft == 0.0f)
        return;
    
    timeLeft -= _dt;

    if (timeLeft <= 0.0f){
        timeLeft = 0.0f;
        goldCount--;

        if (goldCount > 0)
            timeLeft = processingTime;

        Vector3 spawnOffset = entity.HasComponent<Transform>()
        ? entity.GetComponent<Transform>().GetGlobalPosition() + Vector3(0.0f, 0.0f, 1.0f)
        : Vector3(0.0f);

        for (Entity *root : entity.scene.Instantiate(dropPrefab))
        {
            if (root != nullptr && root->HasComponent<Transform>())
                root->GetComponent<Transform>().position += spawnOffset;
        }
    }
}

std::string CoiningPress::GetMessage(const InteractionContext &_context)
{
    (void)_context;
    std::string message = "Left Click to add Gold.";

    if (timeLeft > 0.0f)
        message = "Processing 1 of " + std::to_string(goldCount) + "\n" + message;

    return message;
}

bool CoiningPress::HandleInteraction(const InteractionContext &_context)
{
    InputManager& input = entity.scene.GetInputManager();
    if (!input.LeftClickReleased())
        return false;

    if (_context.interactingEntity == nullptr)
        return false;

    if (SuperPupUtilities::Inventory* inventory = _context.interactingEntity->GetScript<SuperPupUtilities::Inventory>())
    {
        if (inventory->Remove("Gold", 1))
        {
            goldCount++;

            if (timeLeft == 0.0f)
                timeLeft = processingTime;

            return true;
        }
    }

    return false;
}
