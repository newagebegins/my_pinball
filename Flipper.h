#ifndef FLIPPER_H
#define FLIPPER_H

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

class Flipper
{
public:
    static constexpr float r0{ 1.1f };
    static constexpr float r1{ 0.7f };

    static constexpr float minAngle{ glm::radians(-38.0f) };
    static constexpr float maxAngle{ glm::radians(33.0f) };

    Flipper(glm::vec2 position, bool isLeft);
    void activate();
    void deactivate();
    void update(float dt);
    const glm::mat3& getTransform() const;

private:
    static constexpr float maxAngularVelocity{ 2.0f * glm::pi<float>() * 4.0f };

    glm::mat3 m_transform{ glm::mat3{ 1.0f } };
    const glm::vec2 m_position{ glm::vec2{ 0.0f } };
    float m_orientation{ 0.0f };
    const float m_scaleX{ 1.0f };
    float m_angularVelocity{ -maxAngularVelocity };

    void updateTransform();
};

#endif
