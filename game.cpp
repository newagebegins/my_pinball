#include "game.h"

#include <glm/glm.hpp> // for glm types
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp> // for glm::translate(), glm::rotate(), glm::scale()

static void updateFlipperTransform(Flipper& flipper)
{
    flipper.transform = glm::mat3{ 1.0f };
    flipper.transform = glm::translate(flipper.transform, flipper.position);
    flipper.transform = glm::rotate(flipper.transform, flipper.orientation);
    flipper.transform = glm::scale(flipper.transform, { flipper.scaleX, 1.0f });
}

Scene makeScene()
{
    Scene scene{};

    scene.circle = { {0.0f, 0.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ -2.0f };
    scene.flippers[0].position = { -flipperX, flipperY };
    scene.flippers[0].scaleX = 1.0f;
    scene.flippers[1].position = { flipperX, flipperY };
    scene.flippers[1].scaleX = -1.0f;

    updateFlipperTransform(scene.flippers[0]);
    updateFlipperTransform(scene.flippers[1]);

    return scene;
}

void update(std::uint8_t input)
{
    if (input & BUTTON_L)
    {
        // do stuff
    }
    if (input & BUTTON_R)
    {
        // do stuff
    }
}
