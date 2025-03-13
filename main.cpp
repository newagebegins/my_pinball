#include "Circle.h"
#include "CircleRenderer.h"
#include "DefaultShader.h"
#include "Flipper.h"
#include "FlipperRenderer.h"
#include "LineSegmentRenderer.h"

#include <glm/glm.hpp> // for glm types
#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()
#include <glm/ext/scalar_constants.hpp> // for glm::pi()

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp> // for glm::translate(), glm::rotate(), glm::scale()

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cmath> // for std::sin(), std::cos(), std::acos()
#include <cstdint> // for std::uint8_t
#include <cstdlib> // for std::exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <vector>

static void errorCallback(int /*error*/, const char* description)
{
    std::cerr << "GLFW error: " << description << '\n';
}

void APIENTRY glDebugOutput(
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

struct Scene
{
    Circle circle;
    std::vector<Flipper> flippers;
    std::vector<glm::vec2> lines;
};

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

void addLineSegment(std::vector<glm::vec2>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back(p0);
    verts.push_back(p1);
}

void addMirroredLineSegments(std::vector<glm::vec2>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back(p0);
    verts.push_back(p1);

    verts.emplace_back(-p0.x, p0.y);
    verts.emplace_back(-p1.x, p1.y);
}

Scene scene{};

constexpr float worldL{ -35.0f };
constexpr float worldR{ 35.0f };
constexpr float worldT{ 70.0f };
constexpr float worldB{ 0.0f };

DefaultShader* defShader{};
CircleRenderer* circleRenderer{};
FlipperRenderer* flipperRenderer{};
LineSegmentRenderer* lineSegmentRenderer{};

void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
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

void render()
{
    const glm::mat3 identity{ 1.0f };
    const glm::mat4 projection{ glm::ortho(worldL, worldR, worldB, worldT, -1.0f, 1.0f) };

    defShader->use();
    defShader->setView(identity);
    defShader->setProjection(projection);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    circleRenderer->render(scene.circle, defShader);

    for (const auto& flipper : scene.flippers)
    {
        flipperRenderer->render(flipper, defShader);
    }

    lineSegmentRenderer->render(defShader);
}

void windowRefreshCallback(GLFWwindow* window)
{
    render();
    glfwSwapBuffers(window);
}


struct Line
{
    glm::vec2 p; // point on the line
    glm::vec2 d; // direction
};

void addLine(std::vector<glm::vec2>& verts, const Line& l)
{
    constexpr float len{ 100.0f };
    verts.push_back(l.p + l.d*len);
    verts.push_back(l.p - l.d*len);
}

void update(float dt, std::uint8_t input)
{
    if (input & BUTTON_L)
    {
        scene.flippers[0].activate();
    }
    else
    {
        scene.flippers[0].deactivate();
    }

    if (input & BUTTON_R)
    {
        scene.flippers[1].activate();
    }
    else
    {
        scene.flippers[1].deactivate();
    }

    for (auto& flipper : scene.flippers)
    {
        flipper.update(dt);
    }
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

    // Ball's radius is 1.0f, everything is measured relative to that

    scene.circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    scene.flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    scene.flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    // Angled wall right near the flipper
    {
        constexpr float angle{ glm::radians(180.0f) + Flipper::minAngle };
        constexpr float width{ 11.0f };
        const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };
        const glm::vec2 p1{ p0 + glm::vec2{std::cos(angle), std::sin(angle)} * width };
        addMirroredLineSegments(scene.lines, p0, p1);
        const glm::vec2 p2{ p1 + glm::vec2{ 0.0f, 13.0f } };
        addMirroredLineSegments(scene.lines, p1, p2);
    }

    // Draw a border that represents the gameplay area
    {
        constexpr float d{ 0.1f };

        // bottom
        addLineSegment(scene.lines, { worldL+d, worldB+d }, { worldR-d, worldB+d });
        // top
        addLineSegment(scene.lines, { worldL+d, worldT-d }, { worldR-d, worldT-d });
        // left
        addLineSegment(scene.lines, { worldL+d, worldB+d }, { worldL+d, worldT-d });
        // right
        addLineSegment(scene.lines, { worldR-d, worldB+d }, { worldR-d, worldT-d });
    }

    addLine(scene.lines, { { 0.0f, 0.0f }, { 0.5f, 0.5f }});

    defShader = new DefaultShader();
    circleRenderer = new CircleRenderer();
    flipperRenderer = new FlipperRenderer();
    lineSegmentRenderer = new LineSegmentRenderer(scene.lines);

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

        update(dt, input);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete lineSegmentRenderer;
    delete flipperRenderer;
    delete circleRenderer;
    delete defShader;

    return EXIT_SUCCESS;
}
