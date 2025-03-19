#ifndef GAME_H
#define GAME_H

#include "DefaultVertex.h"

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <cassert>
#include <vector>

constexpr float pi{ glm::pi<float>() };
constexpr float twoPi{ 2.0f * pi };

inline glm::vec2 perp(glm::vec2 v)
{
    return {-v.y, v.x};
}

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

struct Circle
{
    glm::vec2 center;
    float radius;
};

struct Arc
{
    glm::vec2 p;
    float r;
    float start;
    float end;

    Arc(glm::vec2 P, float R, float S, float E)
        : p{P}, r{R}, start{S}, end{E}
    {
        assert(0.0f <= start && start < twoPi);
        assert(0.0f <= end && end < twoPi);    
    }
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
    const glm::vec2 m_position{ glm::vec2{ 0.0f } };
    float m_orientation{ 0.0f };
    const float m_scaleX{ 1.0f };
    float m_angularVelocity{ -maxAngularVelocity };

    void updateTransform()
    {
        m_transform = glm::mat3{ 1.0f };
        m_transform = glm::translate(m_transform, m_position);
        m_transform = glm::rotate(m_transform, m_orientation * m_scaleX);
        m_transform = glm::scale(m_transform, { m_scaleX, 1.0f });
    }
};

struct Game
{
    std::vector<Circle> circles{};
    std::vector<Flipper> flippers{};
    std::vector<DefaultVertex> lines{};

    Game();
    void update(float dt, std::uint8_t input);
};

#endif
