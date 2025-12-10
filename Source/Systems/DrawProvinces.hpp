#include <flecs.h>

void DoRenderTileMapSystem(const flecs::world &ecs);

enum class MapMode
{
    Geographic,
    Political,
    Cultural,
    Distance,
    Diplomatic,
};