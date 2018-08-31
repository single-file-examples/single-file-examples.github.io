/*
 * == COMPILING ==
 *
 * To compile this file with MinGW on Windows, run the following command:
 *     g++ -Wall -o shader-and-buffer-example.exe shader-and-buffer-example.cpp -lopengl32 -lgdi32 -Iinclude
 *
 * == DESCRIPTION ==
 *
 * This example shows how to render a colored square from an angle with a GL buffer, a GL shader and matrices.
 *
 * == CONTENT ==
 *
 * This file is seperated into multiple sections:
 *  - Math code
 *  - Shader code
 *  - Buffer code
 *  - Application code
 *  - Backend code
 */

#include <functional>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cmath>
#include "glad.c"

#define EXAMPLE_NAME __FILE__

/******************************************************************************\
 * MATH CODE
\******************************************************************************/

namespace glm
{

struct vec3
{
    vec3() : x(0), y(0), z(0) { }
    vec3(float v) : x(v), y(v), z(v) { }
    vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }

    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };

    float const &operator[] (int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;

        return z;
    }

    float &operator[] (int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;

        return z;
    }
};

vec3 operator + (vec3 const &v1, vec3 const &v2)
{
    return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

vec3 operator - (vec3 const &v1, vec3 const &v2)
{
    return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

vec3 cross(vec3 const &v1, vec3 const &v2)
{
    return vec3(v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x);
}

float length(vec3 const &v)
{
    return float(sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}

vec3 normal(vec3 const &v)
{
    auto l = length(v);
    return vec3(v.x / l, v.y / l, v.z / l);
}

struct vec4
{
    vec4() : x(0), y(0), z(0), w(0) { }
    vec4(float v) : x(v), y(v), z(v), w(v) { }
    vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }

    union { float x; float r; };
    union { float y; float g; };
    union { float z; float b; };
    union { float w; float a; };

    float const &operator[] (int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        return w;
    }

    float &operator[] (int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        return w;
    }
};

struct mat4
{
    mat4() { }
    mat4(float v)
    {
        // Identity
        values[0].x = v;
        values[1].y = v;
        values[2].z = v;
        values[3].w = v;
    }
    mat4(vec4 const &v0, vec4 const &v1, vec4 const &v2, vec4 const &v3)
    {
        values[0] = v0;
        values[1] = v1;
        values[2] = v2;
        values[3] = v3;
    }

    vec4 values[4];

    vec4 const &operator [] (int index) const
    {
        return values[index];
    }

    vec4 &operator [] (int index)
    {
        return values[index];
    }
};

vec4 operator * (mat4 const &m, vec4 const &v)
{
    return vec4(
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3]
    );
}

vec4 operator * (vec4 const &v, mat4 const &m)
{
    return vec4(
        v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + v[3] * m[0][3],
        v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + v[3] * m[1][3],
        v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + v[3] * m[2][3],
        v[0] * m[3][0] + v[1] * m[3][1] + v[2] * m[3][2] + v[3] * m[3][3]
    );
}

mat4 operator * (mat4 const &m1, mat4 const &m2)
{
    vec4 X = m1 * m2[0];
    vec4 Y = m1 * m2[1];
    vec4 Z = m1 * m2[2];
    vec4 W = m1 * m2[3];

    return mat4(X, Y, Z, W);
}

float radians(float degrees)
{
    return degrees * 0.01745329251994329576923690768489f;
}

float const *value_ptr(mat4 const &m)
{
    return &m.values[0].x;
}

mat4 perspective(float fovy, float aspect, float zNear, float zFar)
{
    float const tanHalfFovy = tan(fovy / static_cast<float>(2));

    mat4 m(static_cast<float>(0));
    m[0][0] = static_cast<float>(1) / (aspect * tanHalfFovy);
    m[1][1] = static_cast<float>(1) / (tanHalfFovy);
    m[2][2] = - (zFar + zNear) / (zFar - zNear);
    m[2][3] = - static_cast<float>(1);
    m[3][2] = - (static_cast<float>(2) * zFar * zNear) / (zFar - zNear);

    return m;
}

mat4 lookAt(vec3 const &eye, vec3 const &target, vec3 const &up)
{
    vec3 zaxis = normal(eye - target);
    vec3 xaxis = normal(cross(up, zaxis));
    vec3 yaxis = cross(zaxis, xaxis);

    mat4 orientation = {
        vec4(xaxis.x, yaxis.x, zaxis.x, 0),
        vec4(xaxis.y, yaxis.y, zaxis.y, 0),
        vec4(xaxis.z, yaxis.z, zaxis.z, 0),
        vec4(  0,       0,       0,     1)
    };

    mat4 translation = {
        vec4(   1,      0,      0,   0),
        vec4(   0,      1,      0,   0),
        vec4(   0,      0,      1,   0),
        vec4(-eye.x, -eye.y, -eye.z, 1)
    };

    return (orientation * translation);
}

std::string to_string(vec4 const &x)
{
    std::stringstream ss;

    ss << "(" << x.x << ", " << x.y << ", " << x.z << ", " << x.w << ")";

    return ss.str();
}

std::string to_string(mat4 const &x)
{
    std::stringstream ss;

    ss << "mat4x4(" << to_string(x.values[0]) << ", "
            << to_string(x.values[1]) << ", "
            << to_string(x.values[2]) << ", "
            << to_string(x.values[3]) << ")";

    return ss.str();
}

}

/******************************************************************************\
 * SHADER CODE
\******************************************************************************/

class ShaderType
{
    GLuint _shaderId;
    GLuint _matrixUniformId;

    std::string _matrixUniformName;
    std::string _vertexAttributeName;
    std::string _colorAttributeName;

public:
    ShaderType()
        : _shaderId(0), _matrixUniformId(0),
          _matrixUniformName("u_matrix"),
          _vertexAttributeName("vertex"), _colorAttributeName("color")
    { }

    virtual ~ShaderType() { }

    GLuint id() const
    {
        return _shaderId;
    }

    void use() const
    {
        glUseProgram(_shaderId);
    }

    bool compileDefaultShader()
    {
        static GLuint defaultShader = 0;

        if (defaultShader == 0)
        {
            std::string const vshader(
                        "#version 150\n"

                        "in vec3 vertex;"
                        "in vec4 color;"

                        "uniform mat4 u_matrix;"

                        "out vec4 f_color;"

                        "void main()"
                        "{"
                        "    gl_Position = u_matrix * vec4(vertex.xyz, 1.0);"
                        "    f_color = color;"
                        "}"
                        );

            std::string const fshader(
                        "#version 150\n"

                        "in vec4 f_color;"
                        "out vec4 color;"

                        "void main()"
                        "{"
                        "   color = f_color;"
                        "}"
                        );

            if (compile(vshader, fshader))
            {
                defaultShader = _shaderId;

                return true;
            }

            return false;
        }

        return true;
    }

    virtual bool compile(std::string const &vertShaderStr, std::string const &fragShaderStr)
    {
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *vertShaderSrc = vertShaderStr.c_str();
        const char *fragShaderSrc = fragShaderStr.c_str();

        GLint result = GL_FALSE;
        GLint logLength;

        // Compile vertex shader
        glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
        glCompileShader(vertShader);

        // Check vertex shader
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> vertShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
            std::cout << &vertShaderError[0] << std::endl;

            return false;
        }

        // Compile fragment shader
        glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
        glCompileShader(fragShader);

        // Check fragment shader
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> fragShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
            std::cout << &fragShaderError[0] << std::endl;

            return false;
        }

        _shaderId = glCreateProgram();
        glAttachShader(_shaderId, vertShader);
        glAttachShader(_shaderId, fragShader);
        glLinkProgram(_shaderId);

        glGetProgramiv(_shaderId, GL_LINK_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetProgramiv(_shaderId, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> programError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetProgramInfoLog(_shaderId, logLength, NULL, &programError[0]);
            std::cout << &programError[0] << std::endl;

            return false;
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        _matrixUniformId = glGetUniformLocation(_shaderId, _matrixUniformName.c_str());

        return true;
    }

    void setupMatrices(glm::mat4 const &matrix)
    {
        use();

        glUniformMatrix4fv(_matrixUniformId, 1, false, glm::value_ptr(matrix));
    }

    void setupAttributes() const
    {
        auto vertexSize = sizeof(glm::vec3) + sizeof(glm::vec4);

        auto vertexAttrib = glGetAttribLocation(_shaderId, _vertexAttributeName.c_str());
        glVertexAttribPointer(GLuint(vertexAttrib), sizeof(glm::vec3) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray(GLuint(vertexAttrib));

        auto colorAttrib = glGetAttribLocation(_shaderId, _colorAttributeName.c_str());
        glVertexAttribPointer(GLuint(colorAttrib), sizeof(glm::vec4) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(glm::vec3)));
        glEnableVertexAttribArray(GLuint(colorAttrib));
    }
};

/******************************************************************************\
 * BUFFER CODE
\******************************************************************************/

class VertexType
{
public:
    glm::vec3 pos;
    glm::vec4 col;
};

class BufferType
{
    int _vertexCount;
    std::vector<VertexType> _verts;
    glm::vec4 _nextColor;
    unsigned int _vertexArrayId;
    unsigned int _vertexBufferId;
    GLenum _drawMode;
    std::map<int, int> _faces;

public:
    BufferType()
        : _vertexCount(0), _vertexArrayId(0), _vertexBufferId(0), _drawMode(GL_TRIANGLES)
    { }

    virtual ~BufferType() { }

    std::vector<VertexType>& verts()
    {
        return _verts;
    }

    BufferType& operator << (VertexType const &vertex)
    {
        _verts.push_back(vertex);
        _vertexCount = _verts.size();

        return *this;
    }

    void setDrawMode(GLenum mode)
    {
        _drawMode = mode;
    }

    void addFace(int start, int count)
    {
        _faces.insert(std::make_pair(start, count));
    }

    int vertexCount() const
    {
        return _vertexCount;
    }

    BufferType& vertex(glm::vec3 const &position)
    {
        _verts.push_back(VertexType({ position, _nextColor }));

        _vertexCount = _verts.size();

        return *this;
    }

    BufferType& color(glm::vec4 const &color)
    {
        _nextColor = color;

        return *this;
    }

    bool setup(ShaderType &shader)
    {
        return setup(_drawMode, shader);
    }

    bool setup(GLenum mode, ShaderType &shader)
    {
        _drawMode = mode;
        _vertexCount = _verts.size();

        glGenVertexArrays(1, &_vertexArrayId);
        glGenBuffers(1, &_vertexBufferId);

        glBindVertexArray(_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);

        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(_verts.size() * sizeof(VertexType)), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, GLsizeiptr(_verts.size() * sizeof(VertexType)), reinterpret_cast<const GLvoid*>(&_verts[0]));

        shader.setupAttributes();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        _verts.clear();

        return true;
    }

    void render()
    {
        glBindVertexArray(_vertexArrayId);
        if (_faces.empty())
        {
            glDrawArrays(_drawMode, 0, _vertexCount);
        }
        else
        {
            for (auto pair : _faces)
            {
                glDrawArrays(_drawMode, pair.first, pair.second);
            }
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void cleanup()
    {
        if (_vertexBufferId != 0)
        {
            glDeleteBuffers(1, &_vertexBufferId);
            _vertexBufferId = 0;
        }
        if (_vertexArrayId != 0)
        {
            glDeleteVertexArrays(1, &_vertexArrayId);
            _vertexArrayId = 0;
        }
    }
};

/******************************************************************************\
* APPLICATION CODE
\******************************************************************************/

static struct {
    glm::mat4 matrix;
    glm::vec3 position;
    ShaderType shader;
    BufferType vertexBuffer;
} State;

bool Startup()
{
    std::cout << EXAMPLE_NAME << " startup()\n";
    
    glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
    
    State.shader.compileDefaultShader();
    
    State.vertexBuffer
        .color(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(-10.0f, -10.0f, 0.0f))   // mint
        .color(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)).vertex(glm::vec3(-10.0f, 10.0f, 0.0f))    // geel
        .color(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, 10.0f, 0.0f))     // paars
        .color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, -10.0f, 0.0f))    // wit
        .setup(GL_TRIANGLE_FAN, State.shader);
    
    return true;
}

void Resize(int width, int height)
{
    std::cout << EXAMPLE_NAME << " resize()\n";
    glViewport(0, 0, width, height);

    // Calculate the projection and view matrix
    State.matrix = glm::perspective(glm::radians(90.0f), float(width) / float(height), 0.1f, 4096.0f) * glm::lookAt(State.position + glm::vec3(12.0f), State.position, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Destroy()
{
    std::cout << EXAMPLE_NAME << " destroy()\n";
}

bool Run()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Select shader
    State.shader.use();

    // Upload projection and view matrix into shader
    State.shader.setupMatrices(State.matrix);

    // Render vertex buffer with selected shader
    State.vertexBuffer.render();
    
    return true; // to keep running
}

class Application
{
public:
    virtual ~Application();
    virtual int Run(std::function<bool()> tick) = 0;
    static Application *Create(std::function<bool()> intialize, std::function<void(int width, int height)> resize, std::function<void()> destroy);
};

int main(int argc, char *argv[])
{
    auto app = Application::Create(Startup, Resize, Destroy);

    return app->Run(Run);
}





/* ---------------------------------------------------------------------------*\
 * BACKEND CODE
 *
 * Next follows the code-under-the-hood to make a single file example work
 * If you don't know what your doing, please leave this as is:)
\* ---------------------------------------------------------------------------*/
#ifdef _WIN32

#include <GL/wglext.h>
#include <windows.h>

Application::~Application() {}

typedef HGLRC (WINAPI * PFNGLXCREATECONTEXTATTRIBS) (HDC hDC, HGLRC hShareContext, const int *attribList);

class Win32Application : public Application
{
    std::function<void(int width, int height)> _resize;
    std::function<void()> _destroy;
    HINSTANCE _hInstance;
    HWND _hWnd;
    HDC _hDC;
    HGLRC _hRC;
    PFNGLXCREATECONTEXTATTRIBS _pfnGlxCreateContext;
    
    virtual void Destroy(const char *errorMessage = nullptr);
    
    LRESULT CALLBACK objectProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK staticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    bool Startup(std::function<bool()> intialize, std::function<void(int width, int height)> resize, std::function<void()> destroy);
    virtual int Run(std::function<bool()> tick);

};

bool Win32Application::Startup(std::function<bool()> intialize, std::function<void(int width, int height)> resize, std::function<void()> destroy)
{
    _resize = resize;
    _destroy = destroy;
    _hInstance = GetModuleHandle(nullptr);
    
    WNDCLASS wc;

    if (GetClassInfo(_hInstance, EXAMPLE_NAME, &wc) == FALSE)
    {
        wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc		= (WNDPROC) Win32Application::staticProc;
        wc.cbClsExtra		= 0;
        wc.cbWndExtra		= 0;
        wc.hInstance		= _hInstance;
        wc.hIcon			= LoadIcon(nullptr, IDI_WINLOGO);
        wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground	= nullptr;
        wc.lpszMenuName		= nullptr;
        wc.lpszClassName	= EXAMPLE_NAME;

        if (RegisterClass(&wc) == FALSE)
        {
            Destroy("Failed to register window class");
            return false;
        }
    }
    
    _hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        EXAMPLE_NAME, EXAMPLE_NAME,
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr,
        _hInstance, (VOID*)this);
    
    if (_hWnd == 0)
    {
        Destroy("Failed to create window");
        return false;
    }
    
    _hDC = GetDC(_hWnd);

    if (_hDC == 0)
    {
        Destroy("Failed to get device context");
        return false;
    }
    
    static	PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
        1,											// Version Number
        PFD_DRAW_TO_WINDOW |						// Format Must Support Window
        PFD_SUPPORT_OPENGL |						// Format Must Support CodeGL
        PFD_DOUBLEBUFFER,							// Must Support Double Buffering
        PFD_TYPE_RGBA,								// Request An RGBA Format
        32,											// Select Our Color Depth
        0, 0, 0, 0, 0, 0,							// Color Bits Ignored
        0,											// No Alpha Buffer
        0,											// Shift Bit Ignored
        0,											// No Accumulation Buffer
        0, 0, 0, 0,									// Accumulation Bits Ignored
        16,											// 16Bit Z-Buffer (Depth Buffer)
        0,											// No Stencil Buffer
        0,											// No Auxiliary Buffer
        PFD_MAIN_PLANE,								// Main Drawing Layer
        0,											// Reserved
        0, 0, 0										// Layer Masks Ignored
    };

    auto pixelFormat = ChoosePixelFormat(_hDC, &pfd);
    if (pixelFormat == false)
    {
        Destroy("Failed to choose pixel format");
        return false;
    }

    if(SetPixelFormat(_hDC, pixelFormat, &pfd) == FALSE)
    {
        Destroy("Failed to set pixel format");
        return false;
    }
    
    _pfnGlxCreateContext = (PFNGLXCREATECONTEXTATTRIBS)wglGetProcAddress("wglCreateContextAttribsARB");
    if (_pfnGlxCreateContext == nullptr)
    {
        _hRC = wglCreateContext(_hDC);
        
        if (_hRC == 0)
        {
            Destroy("Failed to create clasic opengl context (v1.0)");
            return false;
        }
    }
    else
    {
        GLint attribList[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, 0,
            0
        };

        _hRC = _pfnGlxCreateContext(_hDC, 0, attribList);
        if (_hRC == 0)
        {
            Destroy("Failed to create modern opengl context (v3.3)");
            return false;
        }
    }
    
    wglMakeCurrent(_hDC, _hRC);
    
    gladLoadGL();
    
    if (!intialize())
    {
        Destroy("Initialize failed");
        return false;
    }
    
    ShowWindow(_hWnd, SW_SHOW);
    SetForegroundWindow(_hWnd);
    SetFocus(_hWnd);
    
    return true;
}

int Win32Application::Run(std::function<bool()> tick)
{
    while (tick())
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        if (IsWindowVisible(_hWnd) == FALSE)
        {
            break;
        }
        
        SwapBuffers(_hDC);
    }
    
    _destroy();
    
    return 0;
}

void Win32Application::Destroy(const char *errorMessage)
{
    if (errorMessage != nullptr)
    {
        std::cout << errorMessage << std::endl;
    }
    
    _destroy();
    
    UnregisterClass(EXAMPLE_NAME, _hInstance);
    
    if (_hDC != 0)
    {
        ReleaseDC(_hWnd, _hDC);
        _hDC = 0;
    }

    if (_hWnd != 0)
    {
        DestroyWindow(_hWnd);
        _hWnd = 0;
    }
}

LRESULT CALLBACK Win32Application::objectProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SIZE:
        {
            auto width = LOWORD(lParam);
            auto height = HIWORD(lParam);
            
            this->_resize(width, height);
            
            break;
        }
    }
    return DefWindowProc(this->_hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Win32Application::staticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Win32Application *app = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        app = reinterpret_cast <Win32Application*> (((LPCREATESTRUCT)lParam)->lpCreateParams);

        if (app != nullptr)
        {
            app->_hWnd = hWnd;

            SetWindowLong(hWnd, GWL_USERDATA, reinterpret_cast <long> (app));

            return app->objectProc(uMsg, wParam, lParam);
        }
    }
    else
    {
        app = reinterpret_cast <Win32Application*>(GetWindowLong(hWnd, GWL_USERDATA));

        if (app != nullptr)
        {
            return app->objectProc(uMsg, wParam, lParam);
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Application *Application::Create(std::function<bool()> initialize, std::function<void(int width, int height)> resize, std::function<void()> destroy)
{
    static Win32Application app;
    
    if (app.Startup(initialize, resize, destroy))
    {
        return &app;
    }
    
    std::cout << "Create application failed" << std::endl;
    
    exit(0);
    
    return nullptr;
}
#endif // _WIN32

#ifdef __linux__
#endif // __linux__
