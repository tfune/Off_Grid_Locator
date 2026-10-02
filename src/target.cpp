#include "target.h"

static Coordinates target;
static bool targetAvailable = false;

void targetSet(const Coordinates& coordinates)
{
    target = coordinates;
    targetAvailable = true;
}

bool targetGet(Coordinates& coordinates)
{
    if (!targetAvailable)
    {
        return false;
    }

    coordinates = target;
    return true;
}
