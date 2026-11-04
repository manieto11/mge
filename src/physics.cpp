#include "mge/physics.hpp"

#include <iostream>
#include <queue>
#include <vector>

namespace mge
{
    const int MAX_DYNAMIC_CUBES = 1024;
    const int MAX_STATIC_CUBES = 1024;

    const float FIXED_STEP = 0.02f;

    const glm::vec3 gravity =  glm::vec3(0.0f, -9.81f, 0.0f);

    struct dynamic_cube
    {
        glm::vec3 position, size, velocity;
        bool deleted, gravity_disable;
    };

    struct static_cube
    {
        glm::vec3 position, size;
        bool deleted;
    };

    namespace
    {
        bool collision(glm::vec3 minA, glm::vec3 maxA, glm::vec3 p)
        {
            return minA.x > p.x && maxA.x < p.x &&
                   minA.y > p.y && maxA.y < p.y &&
                   minA.z > p.z && maxA.z < p.z;
        }

        bool collision(glm::vec3 minA, glm::vec3 maxA, glm::vec3 minB, glm::vec3 maxB)
        {
            return minA.x < maxB.x && maxA.x > minB.x &&
                   minA.y < maxB.y && maxA.y > minB.y &&
                   minA.z < maxB.z && maxA.z > minB.z;
        }

        void solveCollision(dynamic_cube &a, dynamic_cube &b)
        {
            glm::vec3 minA = a.position;
            glm::vec3 maxA = a.position + a.size;
            glm::vec3 minB = b.position;
            glm::vec3 maxB = b.position + b.size;

            if (!collision(minA, maxA, minB, maxB))
                return;

            float overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
            float overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
            float overlapZ = std::min(maxA.z - minB.z, maxB.z - minA.z);

            if (overlapY <= overlapX && overlapY <= overlapZ)
            {
                if (maxA.y - minB.y < maxB.y - minA.y)
                {
                    a.position.y -= overlapY * 0.5f;
                    b.position.y += overlapY * 0.5f;
                }
                else
                {
                    a.position.y += overlapY * 0.5f;
                    b.position.y -= overlapY * 0.5f;
                }

                a.velocity.y = 0.0f;
                b.velocity.y = 0.0f;
            }
            else if (overlapX <= overlapZ)
            {
                if (maxA.x - minB.x < maxB.x - minA.x)
                {
                    a.position.x -= overlapX * 0.5f;
                    b.position.x += overlapX * 0.5f;
                }
                else
                {
                    a.position.x += overlapX * 0.5f;
                    b.position.x -= overlapX * 0.5f;
                }

                a.velocity.x = 0.0f;
                b.velocity.x = 0.0f;
            }
            else
            {
                if (maxA.z - minB.z < maxB.z - minA.z)
                {
                    a.position.z -= overlapZ * 0.5f;
                    b.position.z += overlapZ * 0.5f;
                }
                else
                {
                    a.position.z += overlapZ * 0.5f;
                    b.position.z -= overlapZ * 0.5f;
                }

                a.velocity.z = 0.0f;
                b.velocity.z = 0.0f;
            }
        }

        void solveCollision(dynamic_cube &a, const static_cube &b)
        {
            glm::vec3 minA = a.position;
            glm::vec3 maxA = a.position + a.size;
            glm::vec3 minB = b.position;
            glm::vec3 maxB = b.position + b.size;

            if (!collision(minA, maxA, minB, maxB))
                return;

            float overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
            float overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
            float overlapZ = std::min(maxA.z - minB.z, maxB.z - minA.z);

            if (overlapY <= overlapX && overlapY <= overlapZ)
            {
                if (overlapY == maxA.y - minB.y)
                    a.position.y -= overlapY;
                else
                    a.position.y += overlapY;

                a.velocity.y = 0.0f;
            }
            else if (overlapX <= overlapZ)
            {
                if (overlapX == maxA.x - minB.x)
                    a.position.x -= overlapX;
                else
                    a.position.x += overlapX;

                a.velocity.x = 0.0f;
            }
            else
            {
                if (overlapZ == maxA.z - minB.z)
                    a.position.z -= overlapZ;
                else
                    a.position.z += overlapZ;

                a.velocity.z = 0.0f;
            }
        }
    }

    std::vector<dynamic_cube> dynamic_cubes;
    std::queue<DynamicCube> deleted_dynamic_cubes;
    std::vector<static_cube> static_cubes;
    std::queue<StaticCube> deleted_static_cubes;

    PhysicsSolver::PhysicsSolver()
    {
        mge::PhysicsSolver::Initialize();
    }

    PhysicsSolver::~PhysicsSolver()
    {
        mge::PhysicsSolver::Finalize();
    }

    void PhysicsSolver::Initialize()
    {
        dynamic_cubes.reserve(MAX_DYNAMIC_CUBES);
        static_cubes.reserve(MAX_STATIC_CUBES);

#ifndef NDEBUG
        std::cout << "Physics solver initialized successfully.\n";
#endif
    }

    void PhysicsSolver::Finalize()
    {
        dynamic_cubes.clear();
        while (!deleted_dynamic_cubes.empty())
            deleted_dynamic_cubes.pop();

        static_cubes.clear();
        while (!deleted_static_cubes.empty())
            deleted_static_cubes.pop();
    }

    void PhysicsSolver::Step()
    {
        for (int i = 0; i < dynamic_cubes.size(); ++i)
        {
            dynamic_cube &d_cube = dynamic_cubes[i];

            if (d_cube.deleted)
                continue;

            if (!d_cube.gravity_disable)
                d_cube.velocity += FIXED_STEP * gravity;

            d_cube.position += FIXED_STEP * d_cube.velocity;

            for (int j = i + 1; j < dynamic_cubes.size(); ++j)
            {
                dynamic_cube &d_cube2 = dynamic_cubes[j];

                if (d_cube2.deleted)
                    continue;

                solveCollision(d_cube, d_cube2);
            }

            for (auto &s_cube : static_cubes)
            {
                if (s_cube.deleted)
                    continue;

                solveCollision(d_cube, s_cube);
            }
        }
    }

    DynamicCube PhysicsSolver::CreateDynamicCube(glm::vec3 position, glm::vec3 size, glm::vec3 velocity)
    {
        DynamicCube dc;

        dynamic_cube cube = {
            .position = position,
            .size = size,
            .velocity = velocity,
            .deleted = false,
        };

        if (deleted_dynamic_cubes.empty())
        {
            dc.id = dynamic_cubes.size();
            dynamic_cubes.push_back(cube);
        }
        else
        {
            dc = deleted_dynamic_cubes.front();
            deleted_dynamic_cubes.pop();
            dynamic_cubes[dc.id] = cube;
        }

        return dc;
    }

    StaticCube PhysicsSolver::CreateStaticCube(glm::vec3 position, glm::vec3 size)
    {
        StaticCube sc;

        static_cube cube = {
            .position = position,
            .size = size,
            .deleted = false,
        };

        if (deleted_static_cubes.empty())
        {
            sc.id = static_cubes.size();
            static_cubes.push_back(cube);
        }
        else
        {
            sc = deleted_static_cubes.front();
            deleted_static_cubes.pop();
            static_cubes[sc.id] = cube;
        }

        return sc;
    }

    void PhysicsSolver::DestroyCube(DynamicCube dc)
    {
        dynamic_cubes[dc.id].deleted = true;
        deleted_dynamic_cubes.emplace(dc);
    }

    void PhysicsSolver::DestroyCube(StaticCube sc)
    {
        static_cubes[sc.id].deleted = true;
        deleted_static_cubes.emplace(sc);
    }

    void PhysicsSolver::SetPosition(DynamicCube dc, glm::vec3 position)
    {
        dynamic_cubes[dc.id].position = position;
    }

    glm::vec3 PhysicsSolver::GetPosition(DynamicCube dc)
    {
        return dynamic_cubes[dc.id].position;
    }

    glm::vec3 PhysicsSolver::GetPosition(StaticCube sc)
    {
        return static_cubes[sc.id].position;
    }

    void PhysicsSolver::SetSize(DynamicCube dc, glm::vec3 size)
    {
        dynamic_cubes[dc.id].size = size;
    }

    glm::vec3 PhysicsSolver::GetSize(DynamicCube dc)
    {
        return dynamic_cubes[dc.id].size;
    }

    glm::vec3 PhysicsSolver::GetSize(StaticCube sc)
    {
        return static_cubes[sc.id].size;
    }

    void PhysicsSolver::SetVelocity(DynamicCube dc, glm::vec3 velocity)
    {
        dynamic_cubes[dc.id].velocity = velocity;
    }

    glm::vec3 PhysicsSolver::GetVelocity(DynamicCube sc)
    {
        return dynamic_cubes[sc.id].velocity;
    }
    glm::vec3 PhysicsSolver::GetVelocity(StaticCube sc)
    {
        return glm::vec3(0.0f);
    }
}
