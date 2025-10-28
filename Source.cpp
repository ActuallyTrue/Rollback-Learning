#include "Network.h"
#include "raylib.h"

Vector2 lines[] = { Vector2 {50, 100}, Vector2 {60, 100}, Vector2 {60, 200} };
const int HISTORY_SIZE = 60;

int LatencyValuesForP1[HISTORY_SIZE];
int LatencyValuesForP2[HISTORY_SIZE];

int AverageLatencyValues[HISTORY_SIZE];

int SyncValuesForP1[HISTORY_SIZE];

Vector2 GraphPointsForP1[HISTORY_SIZE];
Vector2 GraphPointsForP2[HISTORY_SIZE];

Vector2 AveragePingGraphPoints[HISTORY_SIZE];

Vector2 SyncGraphPoints[HISTORY_SIZE];

const int GRAPH_SCALE_X = 15;
const int GRAPH_SCALE_Y = 2;

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

int main() {

    InitializeHost();

    InitWindow(1080, 758, "Rollback Test");

    SetTargetFPS(240);

    SimulationState GameState;

    GameState.Entities[0].Position.x = -200;
    GameState.Entities[0].Position.y = 100;

    GameState.Entities[1].Position.x = 200;
    GameState.Entities[1].Position.y = 100;

    GameState.Inputs[0] = static_cast<unsigned int>(InputCommand::NONE);
    GameState.Inputs[1] = static_cast<unsigned int>(InputCommand::NONE);

    while (WindowShouldClose() == false)
    {
        GameState.Inputs[0] = 0;
        // Input Section
        if (IsGamepadAvailable(0))
        {
            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
            {
                GameState.Inputs[0] |= static_cast<unsigned int>(InputCommand::UP);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            {
                GameState.Inputs[0] |= static_cast<unsigned int>(InputCommand::DOWN);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                GameState.Inputs[0] |= static_cast<unsigned int>(InputCommand::LEFT);
            }

            if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                GameState.Inputs[0] |= static_cast<unsigned int>(InputCommand::RIGHT);
            }
        }

        UpdateEntity(GameState.Inputs[0], GameState.Entities[0]);
        UpdateEntity(GameState.Inputs[1], GameState.Entities[1]);

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
