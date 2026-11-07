#include "mge/input.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "utils.h"

#include <iostream>
#include <vector>

#define MAX_GAMEPADS 16

namespace mge
{
    bool availableGamepad[MAX_GAMEPADS] = {false};

    bool prevKeyValue[(int)mge::KeyboardKey::SIZE] = {GLFW_FALSE};
    bool currKeyValue[(int)mge::KeyboardKey::SIZE] = {GLFW_FALSE};
    std::vector<int> modifiedKeys[2];

    bool prevMouseButtonValue[(int)mge::MouseButton::SIZE] = {GLFW_FALSE};
    bool currMouseButtonValue[(int)mge::MouseButton::SIZE] = {GLFW_FALSE};
    std::vector<int> modifiedMouseButtons[2];

    glm::vec2 prevMousePos = glm::vec2(0);
    glm::vec2 currMousePos = glm::vec2(0);

    void joystick_callback(int jid, int event)
    {
        if (jid >= MAX_GAMEPADS)
        {
#ifndef NDEBUG
            std::cout << "Gamepad " << jid << " not available.\n";
#endif
            return;
        }

        if (event == GLFW_CONNECTED)
        {
            availableGamepad[jid] = glfwJoystickIsGamepad(jid);
#ifndef NDEBUG
            std::cout << "Gamepad " << jid << " connected.\n";
#endif

            mge::Input::GetGamepad(jid).Initialize();
        }
        else if (event == GLFW_DISCONNECTED)
        {
            if (availableGamepad[jid])
                availableGamepad[jid] = false;
#ifndef NDEBUG
            std::cout << "Gamepad " << jid << " disconnected.\n";
#endif
        }
    }

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        modifiedKeys[0].push_back(key);
    }

    void mouse_callback(GLFWwindow *window, int button, int action, int mods)
    {
        if (button >= (int)mge::MouseButton::SIZE)
            return;

        modifiedMouseButtons[0].push_back(button);
    }

    Input::Input()
    {
        Input::Initialize();
    }

    Input::Input(void *window)
    {
        mge::window = (GLFWwindow *)window;
        Initialize();
    }

    Input::~Input()
    {
        Input::Finalize();
    }

    void Input::Initialize()
    {
        if (window == nullptr)
            throw std::runtime_error("Window not initialized.");

        glfwSetJoystickCallback(joystick_callback);

        for (int i = 0; i < MAX_GAMEPADS; ++i)
        {
            availableGamepad[i] = glfwJoystickIsGamepad(i);

            if (availableGamepad[i])
                GetGamepad(i).Initialize();
        }

        modifiedKeys[0].reserve(16);
        modifiedKeys[1].reserve(16);
        glfwSetKeyCallback(window, key_callback);
#ifndef NDEBUG
        std::cout << "Key callback set.\n";
#endif

        modifiedMouseButtons[0].reserve(16);
        modifiedMouseButtons[1].reserve(16);
        glfwSetMouseButtonCallback(window, mouse_callback);
#ifndef NDEBUG
        std::cout << "Mouse button callback set.\n";
#endif
    }

    void Input::Initialize(void *window)
    {
        mge::window = (GLFWwindow *)window;
        Initialize();
    }

    void Input::Update()
    {
        glfwPollEvents();

        for (int i = 0; i < MAX_GAMEPADS; ++i)
            if (availableGamepad[i])
                GetGamepad(i).Update();

        for (const int &key : modifiedKeys[1])
        {
            prevKeyValue[key] = currKeyValue[key];
        }

        modifiedKeys[1].clear();

        for (const int &key : modifiedKeys[0])
        {
            prevKeyValue[key] = currKeyValue[key];
            currKeyValue[key] = glfwGetKey(window, key);
            modifiedKeys[1].push_back(key);
        }

        modifiedKeys[0].clear();

        for (const int &button : modifiedMouseButtons[1])
        {
            prevMouseButtonValue[button] = currMouseButtonValue[button];
        }

        modifiedMouseButtons[1].clear();

        for (const int &button : modifiedMouseButtons[0])
        {
            prevMouseButtonValue[button] = currMouseButtonValue[button];
            currMouseButtonValue[button] = glfwGetMouseButton(window, button);
            modifiedMouseButtons[1].push_back(button);
        }

        modifiedMouseButtons[0].clear();

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        prevMousePos = currMousePos;
        currMousePos = glm::vec2(mouseX, mouseY);
    }

    void Input::Finalize()
    {
        modifiedKeys[0].clear();
        modifiedKeys[1].clear();

        modifiedMouseButtons[0].clear();
        modifiedMouseButtons[1].clear();

        glfwSetJoystickCallback(nullptr);
        glfwSetKeyCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, mouse_callback);
    }

    const Gamepad Input::GetGamepad(int id)
    {
        if (id >= MAX_GAMEPADS or !availableGamepad[id])
            return Gamepad(-1);

        return Gamepad(id);
    }

    bool Input::MouseDown(MouseButton button)
    {
        return currMouseButtonValue[(int)button];
    }

    bool Input::MousePressed(MouseButton button)
    {
        return currMouseButtonValue[(int)button] && !prevMouseButtonValue[(int)button];
    }

    bool Input::MouseReleased(MouseButton button)
    {
        return !currMouseButtonValue[(int)button] && prevMouseButtonValue[(int)button];
    }

    float Input::MouseDeltaX()
    {
        return currMousePos.x - prevMousePos.x;
    }

    float Input::MouseDeltaY()
    {
        return currMousePos.y - prevMousePos.y;
    }

    glm::vec2 Input::MouseDelta()
    {
        return currMousePos - prevMousePos;
    }

    float Input::MousePositionX()
    {
        return currMousePos.x;
    }

    float Input::MousePositionY()
    {
        return currMousePos.y;
    }

    glm::vec2 Input::MousePosition()
    {
        return currMousePos;
    }

    void Input::HideCursor()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void Input::ShowCursor()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Input::DisableCursor()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    bool Input::KeyDown(KeyboardKey key)
    {
        return glfwGetKey(window, (int)key);
    }

    bool Input::KeyPressed(KeyboardKey key)
    {
        return currKeyValue[(int)key] && !prevKeyValue[(int)key];
    }

    bool Input::KeyReleased(KeyboardKey key)
    {
        return !currKeyValue[(int)key] && prevKeyValue[(int)key];
    }
}
