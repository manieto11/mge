#include "mge/physics.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "raylib.h"

#include <iostream>

Camera3D camera = {
    .position = {10.0f, 10.0f, 10.0f},
    .target = {0.0f, 0.0f, 0.0f},
    .up = {0.0f, 1.0f, 0.0f},
    .fovy = 16.0f,
    .projection = CameraProjection::CAMERA_ORTHOGRAPHIC,
};

// Camera3D camera = {
//     .position = {10.0f, 2.0f, 10.0f},
//     .target = {0.0f, 0.0f, 0.0f},
//     .up = {0.0f, 1.0f, 0.0f},
//     .fovy = 90.0f,
//     .projection = CameraProjection::CAMERA_PERSPECTIVE,
// };

const int N = 16;
const float cubeSpeed = 5.0f;
const float sqrt2div2 = 0.7071f;

mge::DynamicCube d_cube;
mge::DynamicCube d_cube2;
mge::StaticCube s_cube;
mge::StaticCube s_cube2;

void render();

int main()
{
    InitWindow(800, 600, "physics");
    SetTargetFPS(60);

    mge::PhysicsSolver::Initialize();

    s_cube = mge::PhysicsSolver::CreateStaticCube(glm::vec3(-N / 2.0f, -1.0f, -N / 2.0f), glm::vec3(N, 1.0f, N));
    s_cube2 = mge::PhysicsSolver::CreateStaticCube(glm::vec3(6.0f, 1.0f, 6.0f), glm::vec3(2.0f, 1.0f, 2.0f));
    d_cube = mge::PhysicsSolver::CreateDynamicCube(glm::vec3(0.0f), glm::vec3(1.0f, 2.0f, 1.0f));
    d_cube2 = mge::PhysicsSolver::CreateDynamicCube(glm::vec3(-5.0f, 5.0f, -5.0f), glm::vec3(3.0f));

    float time = 0.0f;

    std::cout << (IsGamepadAvailable(0) ? "true" : "false") << '\n';

    while (!WindowShouldClose())
    {
        time += GetFrameTime();

        float i = 0.0f;
        float j = 0.0f;

        if (IsGamepadAvailable(0))
        {
            i = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
            j = - GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        }

        if (IsKeyDown(KEY_W))
            j = 1.0f;
        else if (IsKeyDown(KEY_S))
            j = -1.0f;
        if (IsKeyDown(KEY_D))
            i = 1.0f;
        else if (IsKeyDown(KEY_A))
            i = -1.0f;

        while (time > 0.02f)
        {
            mge::PhysicsSolver::Step();
            time -= 0.02f;
        }

        glm::vec3 vel = mge::PhysicsSolver::GetVelocity(d_cube);
        glm::vec2 pVel = glm::vec2(vel.x, vel.z);
        pVel.x = (+i - j) * sqrt2div2;
        pVel.y = (-i - j) * sqrt2div2;

        if (glm::dot(pVel, pVel) > 1.0f)
            pVel = glm::normalize(pVel);

        vel.x = pVel.x * cubeSpeed;
        vel.z = pVel.y * cubeSpeed;

        if (IsKeyPressed(KEY_SPACE))
            vel.y = 10.0f;
        mge::PhysicsSolver::SetVelocity(d_cube, vel);

        render();
    }

    mge::PhysicsSolver::Finalize();

    CloseWindow();
}

void draw_shadow(mge::DynamicCube d_cube)
{
    glm::vec3 dPos = mge::PhysicsSolver::GetPosition(d_cube);
    glm::vec3 dSize = mge::PhysicsSolver::GetSize(d_cube);

    DrawCubeWiresV({dPos.x + dSize.x / 2.0f, 0.01f, dPos.z + dSize.z / 2.0f}, {dSize.x, 0.0f, dSize.z}, DARKGRAY);
}

void draw_cube(mge::DynamicCube d_cube)
{
    glm::vec3 dPos = mge::PhysicsSolver::GetPosition(d_cube);
    glm::vec3 dSize = mge::PhysicsSolver::GetSize(d_cube);

    DrawCubeWiresV({dPos.x + dSize.x / 2.0f, dPos.y + dSize.y / 2.0f, dPos.z + dSize.z / 2.0f}, {dSize.x, dSize.y, dSize.z}, GREEN);

    draw_shadow(d_cube);
}

void draw_cube(mge::StaticCube d_cube)
{
    glm::vec3 dPos = mge::PhysicsSolver::GetPosition(d_cube);
    glm::vec3 dSize = mge::PhysicsSolver::GetSize(d_cube);

    DrawCubeWiresV({dPos.x + dSize.x / 2.0f, dPos.y + dSize.y / 2.0f, dPos.z + dSize.z / 2.0f}, {dSize.x, dSize.y, dSize.z}, BLUE);
}


void render()
{
    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(camera);

    DrawGrid(N, 1.0);

    draw_cube(s_cube);
    draw_cube(s_cube2);
    draw_cube(d_cube);
    draw_cube(d_cube2);

    EndMode3D();

    DrawFPS(10, 10);

    EndDrawing();
}