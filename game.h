#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp> // for glm types
#include <glm/ext/scalar_constants.hpp> // for glm::pi()
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp> // for glm::translate(), glm::rotate(), glm::scale()

#include <cstdint> // for std::uint8_t
#include <vector>

struct Circle
{
    glm::vec2 center;
    float radius;
};

class Flipper
{
public:
    static constexpr float r0{ 1.1f };
    static constexpr float r1{ 0.7f };

    static constexpr float minAngle{ glm::radians(-38.0f) };
    static constexpr float maxAngle{ glm::radians(33.0f) };

    Flipper(glm::vec2 position, bool isLeft)
        : m_position{ position }
        , m_scaleX{ isLeft ? 1.0f : -1.0f }
    {
        updateTransform();
    }

    void activate()
    {
        m_angularVelocity = maxAngularVelocity;
    }

    void deactivate()
    {
        m_angularVelocity = -maxAngularVelocity;
    }

    void update(float dt)
    {
        m_orientation += m_angularVelocity * dt;
        m_orientation = glm::clamp(m_orientation, minAngle, maxAngle);
        updateTransform();
    }

    const glm::mat3& getTransform() const
    {
        return m_transform;
    }

private:
    static constexpr float maxAngularVelocity{ 2.0f * glm::pi<float>() * 4.0f };

    glm::mat3 m_transform{ glm::mat3{ 1.0f } };
    glm::vec2 m_position{ glm::vec2{ 0.0f } };
    float m_orientation{ 0.0f };
    float m_scaleX{ 1.0f };
    float m_angularVelocity{ -maxAngularVelocity };

    void updateTransform()
    {
        m_transform = glm::mat3{ 1.0f };
        m_transform = glm::translate(m_transform, m_position);
        m_transform = glm::rotate(m_transform, m_orientation * m_scaleX);
        m_transform = glm::scale(m_transform, { m_scaleX, 1.0f });
    }
};

struct Scene
{
    Circle circle;
    std::vector<Flipper> flippers;
    std::vector<glm::vec2> lines;
};

Scene makeScene();

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

void update(Scene& scene, float dt, std::uint8_t input);

#endif
