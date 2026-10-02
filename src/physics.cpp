// #include "mge/physics.h"

// #ifdef DIM2D
// #include "box2d/box2d.h"

// b2WorldId worldId;

// bool mge::Physics2D::Init()
// {
//     b2WorldDef worldDef = b2DefaultWorldDef();
//     worldDef.gravity = {0.0f, -10.0f};

//     worldId = b2CreateWorld(&worldDef);

//     return b2World_IsValid(worldId);
// }

// #else
// #include "box3d/box3d.h"

// b3WorldId worldId;

// bool mge::Physics3D::Init()
// {
//     b3WorldDef worldDef = b3DefaultWorldDef();
//     worldDef.gravity = {0.0f, -10.0f, 0.0f};

//     worldId = b3CreateWorld(&worldDef);

//     return b3World_IsValid(worldId);
// }

// #endif