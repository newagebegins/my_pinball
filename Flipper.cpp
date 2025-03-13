#include "Flipper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

Flipper::Flipper(glm::vec2 position, bool isLeft)
    : m_position{ position }
    , m_scaleX{ isLeft ? 1.0f : -1.0f }
{
    updateTransform();
}

void Flipper::activate()
{
    m_angularVelocity = maxAngularVelocity;
}

void Flipper::deactivate()
{
    m_angularVelocity = -maxAngularVelocity;
}

void Flipper::update(float dt)
{
    m_orientation += m_angularVelocity * dt;
    m_orientation = glm::clamp(m_orientation, minAngle, maxAngle);
    updateTransform();
}

const glm::mat3& Flipper::getTransform() const
{
    return m_transform;
}

void Flipper::updateTransform()
{
    m_transform = glm::mat3{ 1.0f };
    m_transform = glm::translate(m_transform, m_position);
    m_transform = glm::rotate(m_transform, m_orientation * m_scaleX);
    m_transform = glm::scale(m_transform, { m_scaleX, 1.0f });
}
