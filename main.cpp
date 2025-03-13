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

constexpr int circleNumVerts{ 32 };

static constexpr int numCircleSegments1{ 16 };
static constexpr int numCircleSegments2{ 8 };
static constexpr int flipperNumVerts{ (numCircleSegments1 + 1) + (numCircleSegments2 + 1) };

static const GLchar* const vertexCode{ R"(
#version 410

layout (location = 0) in vec2 pos;

uniform mat3 model;
uniform mat3 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(view * model * vec3(pos, 1.0), 1.0);
}
)" };

static const GLchar* const fragmentCode{ R"(
#version 410

out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)" };
    
struct RenderData
{
    GLuint program;
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;

    GLuint circleVao;
    GLuint flipperVao;

    GLuint lineSegmentsVao;
    int numLineSegmentVerts;
};

RenderData rd{};

void render(GLFWwindow* window)
{
    int width{};
    int height{};
    glfwGetFramebufferSize(window, &width, &height);
    const float ratio{ width / static_cast<float>(height) };

    const glm::mat3 identity{ 1.0f };
    const glm::mat4 projection{ glm::ortho(worldL, worldR, worldB, worldT, -1.0f, 1.0f) };

    glUseProgram(rd.program);
    glUniformMatrix3fv(rd.viewLoc, 1, GL_FALSE, &identity[0][0]);
    glUniformMatrix4fv(rd.projectionLoc, 1, GL_FALSE, &projection[0][0]);

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

    {
        glm::mat3 model{ 1.0f };
        model = glm::translate(model, scene.circle.center);
        model = glm::scale(model, glm::vec2{ scene.circle.radius });
        glBindVertexArray(rd.circleVao);
        glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &model[0][0]);
        glDrawArrays(GL_LINE_LOOP, 0, circleNumVerts);
    }

    for (const auto& flipper : scene.flippers)
    {
        glBindVertexArray(rd.flipperVao);
        glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &flipper.getTransform()[0][0]);
        glDrawArrays(GL_LINE_LOOP, 0, flipperNumVerts);
    }

    glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &identity[0][0]);
    glBindVertexArray(rd.lineSegmentsVao);
    glDrawArrays(GL_LINES, 0, rd.numLineSegmentVerts);

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

static GLuint createVao(const std::vector<glm::vec2>& verts)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const GLsizeiptr size{ static_cast<GLsizeiptr>(verts.size() * sizeof(verts[0])) };
    glBufferData(GL_ARRAY_BUFFER, size, verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), nullptr);
    glEnableVertexAttribArray(0);
    return vao;
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

    glfwSetWindowRefreshCallback(window, render);

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

    rd.program = createShaderProgram(vertexCode, fragmentCode);
    rd.modelLoc = glGetUniformLocation(rd.program, "model");
    rd.viewLoc = glGetUniformLocation(rd.program, "view");
    rd.projectionLoc = glGetUniformLocation(rd.program, "projection");

    assert(rd.modelLoc >= 0);
    assert(rd.viewLoc >= 0);
    assert(rd.projectionLoc >= 0);

    // Create circle VAO
    {
        std::vector<glm::vec2> verts(circleNumVerts);

        for (std::size_t i{ 0 }; i < circleNumVerts; ++i)
        {
            const float t{ static_cast<float>(i) / circleNumVerts };
            const float angle{ t * 2.0f * glm::pi<float>() };
            verts[i] = { std::cos(angle), std::sin(angle) };
        }

        rd.circleVao = createVao(verts);
    }

    // Create flipper VAO
    {
        constexpr float width{ 8.0f };
        constexpr float d{ width - Flipper::r0 - Flipper::r1 };
        constexpr float cosA{ (Flipper::r0 - Flipper::r1) / d };
        const float a{ std::acos(cosA) };

        std::vector<glm::vec2> vertices(flipperNumVerts);

        std::size_t nextVertex{ 0 };

        for (int i{ 0 }; i <= numCircleSegments1; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments1 };
            const float angle{ a + 2.0f * t * (glm::pi<float>() - a) };
            const float x{ Flipper::r0 * std::cos(angle) };
            const float y{ Flipper::r0 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        for (int i{ 0 }; i <= numCircleSegments2; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments2 };
            const float angle{ -a + t * 2.0f * a };
            const float x{ d + Flipper::r1 * std::cos(angle) };
            const float y{ Flipper::r1 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        assert(nextVertex == flipperNumVerts);

        rd.flipperVao = createVao(vertices);
    }

    // Create line segments VAO
    {
        rd.lineSegmentsVao = createVao(scene.lines);
        rd.numLineSegmentVerts = static_cast<int>(scene.lines.size());
    }

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
        render(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
