#include "mge/physics.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "raylib.h"

#include <iostream>

Camera3D cameras[2] = {{
                           .position = {10.0f, 12.0f, 10.0f},
                           .target = {0.0f, 2.0f, 0.0f},
                           .up = {0.0f, 1.0f, 0.0f},
                           .fovy = 20.0f,
                           .projection = CameraProjection::CAMERA_ORTHOGRAPHIC,
                       },
                       {
                           .position = {10.0f, 10.0f, 10.0f},
                           .target = {0.0f, 0.0f, 0.0f},
                           .up = {0.0f, 1.0f, 0.0f},
                           .fovy = 60.0f,
                           .projection = CameraProjection::CAMERA_PERSPECTIVE,
                       }};

const int N = 16;
const float cubeSpeed = 7.5f;
const float sqrt2div2 = 0.7071f;

mge::DynamicCube d_cube;
mge::DynamicCube d_cube2;
mge::StaticCube s_cube;
mge::StaticCube s_cube2;
mge::StaticCube s_cube3;

int cameraIndex = 0;

void render();

int main()
{
    InitWindow(800, 600, "test physics");
    SetTargetFPS(60);

    mge::PhysicsSolver::Initialize();

    s_cube = mge::PhysicsSolver::CreateStaticCube(glm::vec3(-N / 2.0f, -1.0f, -N / 2.0f), glm::vec3(N, 1.0f, N));
    s_cube2 = mge::PhysicsSolver::CreateStaticCube(glm::vec3(-N / 2.0f, -1.0f, -N / 2.0f - 1.0f), glm::vec3(N, 6.0f, 1.0f));
    s_cube3 = mge::PhysicsSolver::CreateStaticCube(glm::vec3(-N / 2.0f - 1.0f, -1.0f, -N / 2.0f), glm::vec3(1.0f, 6.0f, N));
    d_cube = mge::PhysicsSolver::CreateDynamicCube(glm::vec3(0.0f), glm::vec3(1.0f, 2.0f, 1.0f));
    d_cube2 = mge::PhysicsSolver::CreateDynamicCube(glm::vec3(-5.0f, 5.0f, -5.0f), glm::vec3(3.0f));

    float time = 0.0f;

    while (!WindowShouldClose())
    {
        time += GetFrameTime();

        float i = 0.0f;
        float j = 0.0f;

        if (IsGamepadAvailable(0))
        {
            i = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            j = -GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

            if (std::abs(i) < 0.2f)
                i = 0.0f;
            if (std::abs(j) < 0.2f)
                j = 0.0f;
        }

        if (IsKeyDown(KEY_W))
            j = 1.0f;
        else if (IsKeyDown(KEY_S))
            j = -1.0f;
        if (IsKeyDown(KEY_D))
            i = 1.0f;
        else if (IsKeyDown(KEY_A))
            i = -1.0f;

        glm::vec3 vel = mge::PhysicsSolver::GetVelocity(d_cube);
        glm::vec2 pVel = glm::vec2(vel.x, vel.z);
        pVel.x = (+i - j) * sqrt2div2;
        pVel.y = (-i - j) * sqrt2div2;

        if (glm::dot(pVel, pVel) > 1.0f)
            pVel = glm::normalize(pVel);

        vel.x = pVel.x * cubeSpeed;
        vel.z = pVel.y * cubeSpeed;

        if (IsKeyPressed(KEY_SPACE) or (IsGamepadAvailable(0) and IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))
            vel.y = cubeSpeed;
        mge::PhysicsSolver::SetVelocity(d_cube, vel);

        while (time > 0.02f)
        {
            mge::PhysicsSolver::Step();
            time -= 0.02f;
        }

        if (IsKeyPressed(KEY_ONE))
            cameraIndex = 0;
        else if (IsKeyPressed(KEY_TWO))
            cameraIndex = 1;

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

void draw_position(mge::DynamicCube d_cube, int yPos)
{
    glm::vec3 dPos = mge::PhysicsSolver::GetPosition(d_cube);

    DrawText(TextFormat("Position: {%.2f, %.2f, %.2f}", dPos.x, dPos.y, dPos.z), 10, yPos, 20, BLACK);
}

void render()
{
    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(cameras[cameraIndex]);

    DrawGrid(N, 1.0);

    draw_cube(s_cube);
    draw_cube(s_cube2);
    draw_cube(s_cube3);
    draw_cube(d_cube);
    draw_cube(d_cube2);

    EndMode3D();

    DrawFPS(10, 10);
    draw_position(d_cube, 30);

    EndDrawing();
}