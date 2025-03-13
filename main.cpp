#include "game.h"
#include "renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib> // for std::exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>

static void errorCallback(int /*error*/, const char* description)
{
    std::cerr << "GLFW error: " << description << '\n';
}

void render(GLFWwindow* window);

void windowRefreshCallback(GLFWwindow* window)
{
    render(window);
    glfwSwapBuffers(window);
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

DefaultShader* m_defShader{};
CircleRenderer* m_circleRenderer{};
FlipperRenderer* m_flipperRenderer{};
LineRenderer* m_lineRenderer{};

void render(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    const float ratio = width / (float) height;

    //const glm::mat3 view{ glm::translate(glm::mat3{ 1.0f }, { 0.0f, -30.0f }) };
    const glm::mat3 view{ 1.0f };

    //constexpr float zoom{ 70.0f };
    //const glm::mat4 projection{ glm::ortho(-1.0f * zoom, 1.0f * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f) };
    const glm::mat4 projection{ glm::ortho(worldL, worldR, worldB, worldT, -1.0f, 1.0f) };

    m_defShader->use();
    m_defShader->setView(view);
    m_defShader->setProjection(projection);

    if (width > height)
    {
        int w{ static_cast<GLsizei>(width / ratio) };
        glViewport(width/2 - w/2, 0, w, height);
    }
    else
    {
        int h{ static_cast<GLsizei>(height * ratio) };
        glViewport(0, height/2 - h/2, width, h);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_circleRenderer->render(scene.circle, m_defShader);

    for (const auto& flipper : scene.flippers)
    {
        m_flipperRenderer->render(flipper, m_defShader);
    }

    m_lineRenderer->render(m_defShader);
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

    GLFWwindow* window{ glfwCreateWindow(500, 800, "my_pinball", nullptr, nullptr) };
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

    m_defShader = new DefaultShader{};
    m_circleRenderer = new CircleRenderer{};
    m_flipperRenderer = new FlipperRenderer{};
    m_lineRenderer = new LineRenderer{ scene.lines };

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

        update(scene, dt, input);
        render(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
