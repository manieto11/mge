#if !defined(MGE_PHYSICS_HPP)
#define MGE_PHYSICS_HPP

#include "glm/vec3.hpp"

namespace mge
{
    struct DynamicCube
    {
        unsigned int id;
    };
    struct StaticCube
    {
        unsigned int id;
    };

    class PhysicsSolver
    {
    public:
        PhysicsSolver();
        ~PhysicsSolver();

        static void Initialize();
        static void Finalize();

        static void Step();

        static DynamicCube CreateDynamicCube(glm::vec3 position = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), glm::vec3 velocity = glm::vec3(0.0f));
        static StaticCube CreateStaticCube(glm::vec3 position = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f));

        static void DestroyCube(DynamicCube id);
        static void DestroyCube(StaticCube id);

        static void SetPosition(DynamicCube dc, glm::vec3 position);
        static glm::vec3 GetPosition(DynamicCube dc);
        static glm::vec3 GetPosition(StaticCube sc);

        static void SetSize(DynamicCube dc, glm::vec3 size);
        static glm::vec3 GetSize(DynamicCube dc);
        static glm::vec3 GetSize(StaticCube sc);

        static void SetVelocity(DynamicCube dc, glm::vec3 velocity);
        static glm::vec3 GetVelocity(DynamicCube sc);
        static glm::vec3 GetVelocity(StaticCube sc);
    };
} // namespace mge

#endif