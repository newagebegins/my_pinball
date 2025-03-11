#include <glm/glm.hpp>                   // for glm types
#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp> // for glm::translate(), glm::rotate(), glm::scale()

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cmath> // for std::sin(), std::cos(), std::acos()
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

GLuint createShaderProgram(const GLchar* vertexCode, const GLchar* fragmentCode)
{
    GLint success{};
    GLchar infoLog[512];

    const GLuint vs{ glCreateShader(GL_VERTEX_SHADER) };
    glShaderSource(vs, 1, &vertexCode, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Vertex shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    const GLuint fs{ glCreateShader(GL_FRAGMENT_SHADER) };
    glShaderSource(fs, 1, &fragmentCode, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Fragment shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    const GLuint program{ glCreateProgram() };
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    glDeleteShader(fs);
    glDeleteShader(vs);

    return program;
}

class DefaultShader
{
public:
    DefaultShader()
        : m_program{ createShaderProgram(vertexCode, fragmentCode) }
        , m_modelLoc{ glGetUniformLocation(m_program, "model") }
        , m_projectionLoc{ glGetUniformLocation(m_program, "projection") }
    {
        assert(m_modelLoc >= 0);
        assert(m_projectionLoc >= 0);
    }

    void use() const
    {
        glUseProgram(m_program);
    }

    void setModel(const glm::mat3& model) const
    {
        glUniformMatrix3fv(m_modelLoc, 1, GL_FALSE, &model[0][0]);
    }

    void setProjection(const glm::mat4& projection) const
    {
        glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, &projection[0][0]);
    }

private:
    const GLuint m_program;
    const GLint m_modelLoc;
    const GLint m_projectionLoc;

    inline static const GLchar* const vertexCode{ R"(
#version 460

layout (location = 0) in vec2 pos;

uniform mat3 model;
uniform mat4 projection;

void main()
{
	gl_Position = projection * vec4(model * vec3(pos, 1.0), 1.0);
}
)" };

    inline static const GLchar* const fragmentCode{ R"(
#version 460

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)" };
};

struct Circle
{
    glm::vec2 center;
    float radius;
};

class CircleRenderer
{
public:
    CircleRenderer()
    {
        glm::vec2 verts[numVerts];

        for (int i{ 0 }; i < numVerts; ++i)
        {
            const float t{ static_cast<float>(i) / numVerts };
            const float angle{ t * 2.0f * glm::pi<float>() };
            verts[i] = { std::cos(angle), std::sin(angle) };
        }

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        GLuint vbo{};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), nullptr);
        glEnableVertexAttribArray(0);
    }

    void render(const Circle& c, const DefaultShader& s)
    {
        glm::mat3 model{ glm::mat3(1.0f) };
        model = glm::translate(model, c.center);
        model = glm::scale(model, glm::vec2(c.radius));
        glBindVertexArray(m_vao);
        s.setModel(model);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }

private:
    static constexpr int numVerts{ 32 };
    GLuint m_vao;
};

struct Flipper
{
    glm::mat3 transform;
    glm::vec2 position;
    float orientation;
    float scaleX;
};

class FlipperRenderer
{
public:
    FlipperRenderer()
    {
        constexpr float r0{ 1.1f };
        constexpr float r1{ 0.7f };
        constexpr float width{ 8.0f };
        constexpr float d{ width - r0 - r1 };
        constexpr float cosA{ (r0 - r1) / d };
        const float a{ std::acos(cosA) };

        glm::vec2 vertices[numVerts];

        int nextVertex{ 0 };

        for (int i{ 0 }; i <= numCircleSegments1; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments1 };
            const float angle{ a + 2.0f * t * (glm::pi<float>() - a) };
            const float x{ r0 * std::cos(angle) };
            const float y{ r0 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        for (int i{ 0 }; i <= numCircleSegments2; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments2 };
            const float angle{ -a + t * 2.0f * a };
            const float x{ d + r1 * std::cos(angle) };
            const float y{ r1 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        assert(nextVertex == numVerts);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        GLuint vbo{};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), nullptr);
        glEnableVertexAttribArray(0);
    }

    void render(const Flipper& flipper, const DefaultShader& s)
    {
        glBindVertexArray(m_vao);
        s.setModel(flipper.transform);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }
private:
    static constexpr int numCircleSegments1{ 16 };
    static constexpr int numCircleSegments2{ 8 };
    static constexpr int numVerts{ (numCircleSegments1 + 1) + (numCircleSegments2 + 1) };
    
    GLuint m_vao;
};

void updateFlipperTransform(Flipper& flipper)
{
    flipper.transform = glm::mat3{ 1.0f };
    flipper.transform = glm::translate(flipper.transform, flipper.position);
    flipper.transform = glm::rotate(flipper.transform, flipper.orientation);
    flipper.transform = glm::scale(flipper.transform, { flipper.scaleX, 1.0f });
}

struct Scene
{
    Circle circle;
    Flipper flippers[2];
};

class Renderer
{
public:
    Renderer()
    {
        constexpr float zoom{ 32.0f };
        const glm::mat4 projection{ glm::ortho(-1.0f * zoom, 1.0f * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f) };

        m_defShader.use();
        m_defShader.setProjection(projection);
    }

    void render(const Scene& scene)
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_circleRenderer.render(scene.circle, m_defShader);

        for (const auto& flipper : scene.flippers)
        {
            m_flipperRenderer.render(flipper, m_defShader);
        }
    }

private:
    DefaultShader m_defShader;
    CircleRenderer m_circleRenderer;
    FlipperRenderer m_flipperRenderer;
};

int main()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

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

    Renderer renderer;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        renderer.render(scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
