#include "Network.h"
#include "raylib.h"
#include <plog/Initializers/ConsoleInitializer.h>
#include "plog/Formatters/TxtFormatter.h"

enum class NetworkState
{
    None,
    Host,
    Client
};

//Stores the entire input state for a given frame and player.
//These are all a power of 2 so that bitwise or operations can be done with them
enum class InputCommand : unsigned int
{
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 4,
    RIGHT = 8,
};

/*
 * The complete game state for an entity in the game.
 * The state for an entity needs to be synchronized across the network for players to see the same result
 * on each other's screens. We of course don't use replication, but deterministic lock-step to do this.
 */
struct EntityState {
    Vector2 Position = {0,0};
    Vector2 Velocity = {0,0};;
};

/*
 * This is the complete state of our game. When these states match each other on both game clients, both players
 * will see the same thing on their screen. For a given frame, the next game state will entirely depend on the
 * memory in this struct. it cannot depend on anything external or risk desynchronizing the two game clients.
 */
struct SimulationState {
    EntityState Entities[2];
    unsigned int Inputs[2];
};

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 768;

void DrawEntity(const EntityState& State, Color color)
{
    DrawCircle(State.Position.x + SCREEN_WIDTH / 2, SCREEN_HEIGHT - State.Position.y, 100, color);
}

void UpdateEntity(unsigned int input, EntityState& entity)
{
    //Movement Control
    entity.Velocity.x = 0;
    entity.Velocity.y = 0;

    if ((input & static_cast<unsigned int>(InputCommand::RIGHT)))
    {
        entity.Velocity.x = 2;
    }

    if ((input & static_cast<unsigned int>(InputCommand::LEFT)))
    {
        entity.Velocity.x = -2;
    }

    if ((input & static_cast<unsigned int>(InputCommand::UP)))
    {
        entity.Velocity.y = 2;
    }

    if ((input & static_cast<unsigned int>(InputCommand::DOWN)))
    {
        entity.Velocity.y = -2;
    }

    //Handle Physics
    entity.Position.x += entity.Velocity.x;
    entity.Position.y += entity.Velocity.y;
}

void UpdateSimulation(SimulationState& GameState)
{
    UpdateEntity(GameState.Inputs[0], GameState.Entities[0]);
    UpdateEntity(GameState.Inputs[1], GameState.Entities[1]);
}

int main() {
    plog::init<plog::TxtFormatter>(plog::verbose, plog::streamStdOut); // logs error and above to stderr
    NetworkState NetState = NetworkState::None;

    InitWindow(1080, 758, "Rollback Test");

    SetTargetFPS(240);

    SimulationState GameState;

    //Set starting position of entities
    GameState.Entities[0].Position.x = -200;
    GameState.Entities[0].Position.y = 100;

    GameState.Entities[1].Position.x = 200;
    GameState.Entities[1].Position.y = 100;

    GameState.Inputs[0] = static_cast<unsigned int>(InputCommand::NONE);
    GameState.Inputs[1] = static_cast<unsigned int>(InputCommand::NONE);

    while (WindowShouldClose() == false)
    {
        if (NetState == NetworkState::Host)
        {
            UpdateNetworkHost();
        } else if (NetState == NetworkState::Client)
        {
            UpdateNetworkClient();
        }

        //Only check for starting host/client when the network isn't initialized already
        if (NetState == NetworkState::None)
        {
            if (IsKeyDown(KEY_F1))
            {
                if (InitializeHost())
                {
                    NetState = NetworkState::Host;
                }
            }

            if (IsKeyDown(KEY_F2))
            {
                if (InitializeClient())
                {
                    NetState = NetworkState::Client;
                }
            }
        }

        GameState.Inputs[0] = 0;
        GameState.Inputs[1] = 0;

        // UpdateInput
        if (IsWindowFocused() && IsGamepadAvailable(0))
        {
            int Entity = (NetState == NetworkState::Client) ? 1 : 0;

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
            {
                GameState.Inputs[Entity] |= static_cast<unsigned int>(InputCommand::UP);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            {
                GameState.Inputs[Entity] |= static_cast<unsigned int>(InputCommand::DOWN);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                GameState.Inputs[Entity] |= static_cast<unsigned int>(InputCommand::LEFT);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                GameState.Inputs[Entity] |= static_cast<unsigned int>(InputCommand::RIGHT);
            }
        }

        UpdateSimulation(GameState);

        //Drawing
        {
            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawEntity(GameState.Entities[0], RED);
            DrawEntity(GameState.Entities[1], BLUE);
            EndDrawing();
        }
    }
}
