#include "Constants.h"
#include "DefaultShader.h"
#include "FlipperRenderer.h"
#include "Game.h"
#include "LineSegmentRenderer.h"

#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>

class CircleRenderer
{
public:
    CircleRenderer() : m_vao{ DefaultShader::createVao(makeVerts()) }
    {}

    CircleRenderer(const CircleRenderer&) = delete;

    void render(const Circle& c, const DefaultShader& s) const
    {
        glm::mat3 model{ 1.0f };
        model = glm::translate(model, c.center);
        model = glm::scale(model, glm::vec2{ c.radius });
        s.setModel(model);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }
private:
    static constexpr int numVerts{ 64 };

    static std::vector<DefaultVertex> makeVerts()
    {
        std::vector<DefaultVertex> verts(numVerts);

        for (std::size_t i{ 0 }; i < numVerts; ++i)
        {
            const float t{ static_cast<float>(i) / numVerts };
            const float angle{ t * 2.0f * glm::pi<float>() };
            verts[i] = {
                { std::cos(angle), std::sin(angle) },
                { 1.0f, 1.0f, 1.0f },
            };
        }

        return verts;
    }

    const GLuint m_vao{};
};

class Renderer
{
public:
    Renderer(const Game& game)
        : m_lineSegmentRenderer{ game.lines }
    {
        const glm::mat3 identity{ 1.0f };
        const glm::mat4 projection{ glm::ortho(Constants::worldL, Constants::worldR, Constants::worldB, Constants::worldT, -1.0f, 1.0f) };

        m_defShader.use();
        m_defShader.setView(identity);
        m_defShader.setProjection(projection);
    }

    Renderer(const Renderer&) = delete;
    void render(const Game& game) const
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (const auto& c : game.circles)
        {
            m_circleRenderer.render(c, m_defShader);
        }

        for (const auto& flipper : game.flippers)
        {
            m_flipperRenderer.render(flipper, m_defShader);
        }

        m_lineSegmentRenderer.render(m_defShader);
    }
private:
    DefaultShader m_defShader{};
    CircleRenderer m_circleRenderer{};
    FlipperRenderer m_flipperRenderer{};
    LineSegmentRenderer m_lineSegmentRenderer;
};

Game game{};
Renderer* renderer{};

static void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei /*length*/,
    const GLchar* message,
    const void* /*userParam*/)
{
    // Skip uninteresting messages
    if (id == 131185) // Buffer object will use VIDEO memory as the source for buffer object operations
        return;

    std::cerr << "OpenGL debug message (" << id << "): " << message << '\n';

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
    default:                              std::cerr << "Source: ???"; break;
    }
    std::cerr << '\n';

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
    default:                                std::cerr << "Type: ???"; break;
    }
    std::cerr << '\n';

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
    default:                             std::cerr << "Severity: ???"; break;
    }
    std::cerr << "\n\n";
}

static void errorCallback(int /*error*/, const char* description)
{
    std::cerr << "GLFW error: " << description << '\n';
}

static void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    if (width > height)
    {
        int w{ height };
        glViewport(width/2 - w/2, 0, w, height);
    }
    else
    {
        int h{ width };
        glViewport(0, height/2 - h/2, width, h);
    }
}

static void windowRefreshCallback(GLFWwindow* window)
{
    renderer->render(game);
    glfwSwapBuffers(window);
}

int main()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window{ glfwCreateWindow(800, 800, "my_pinball", nullptr, nullptr) };
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return EXIT_FAILURE;
    }

    if (GLAD_GL_KHR_debug)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowRefreshCallback(window, windowRefreshCallback);

    renderer = new Renderer(game);

    while (!glfwWindowShouldClose(window))
    {
        static float prevTime{ static_cast<float>(glfwGetTime()) };
        const float currentTime{ static_cast<float>(glfwGetTime()) };
        const float dt = currentTime - prevTime;
        prevTime = currentTime;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        std::uint8_t input{};

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            input |= BUTTON_L;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            input |= BUTTON_R;
        }

        game.update(dt, input);
        renderer->render(game);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete renderer;

    return EXIT_SUCCESS;
}
