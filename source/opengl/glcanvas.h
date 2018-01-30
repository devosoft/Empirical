#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "glwrap.h"
#include "math/region.h"
#include "shaders.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string>
#else
#include <thread>
#endif

#include <functional>
#include <vector>

namespace emp {
  namespace opengl {

#ifdef __EMSCRIPTEN__
    unsigned int getWindowWidth() {
      return EM_ASM_INT({ return window.innerWidth; });
    }

    unsigned int getWindowHeight() {
      return EM_ASM_INT({ return window.innerHeight; });
    }
#endif

    class MouseEvent {
      public:
      struct Button {
        enum Id : int {
          Left = GLFW_MOUSE_BUTTON_LEFT,
          Right = GLFW_MOUSE_BUTTON_RIGHT,
          Middle = GLFW_MOUSE_BUTTON_MIDDLE,
          Button1 = GLFW_MOUSE_BUTTON_1,
          Button2 = GLFW_MOUSE_BUTTON_2,
          Button3 = GLFW_MOUSE_BUTTON_4,
          Button4 = GLFW_MOUSE_BUTTON_3,
          Button5 = GLFW_MOUSE_BUTTON_5,
          Button6 = GLFW_MOUSE_BUTTON_6,
          Button7 = GLFW_MOUSE_BUTTON_7,
          Button8 = GLFW_MOUSE_BUTTON_8
        } button;
        bool wasPressed;
        bool pressed;
      };

      class Modifiers {
        private:
        int mod;

        public:
        constexpr Modifiers(int mod = 0) : mod(mod) {}
        constexpr Modifiers(const Modifiers&) = default;
        constexpr Modifiers(Modifiers&&) = default;
        constexpr Modifiers& operator=(const Modifiers&) = default;
        constexpr Modifiers& operator=(Modifiers&&) = default;

        constexpr bool shift() const { return (mod & GLFW_MOD_SHIFT) != 0; }
        constexpr bool control() const { return (mod & GLFW_MOD_CONTROL) != 0; }
        constexpr bool alt() const { return (mod & GLFW_MOD_ALT) != 0; }
        constexpr bool super() const { return (mod & GLFW_MOD_SUPER) != 0; }
      };

      Vec2d position;
      Button button;
      Modifiers modifiers;

      constexpr MouseEvent()
        : position(0, 0), button{Button::Left, false, false} {}
    };

    std::ostream& operator<<(std::ostream& out,
                             const MouseEvent::Modifiers& mod) {
      if (mod.shift()) out << "Shift";
      if (mod.control()) out << "Ctrl";
      if (mod.alt()) out << "Alt";
      if (mod.super()) out << "Sup";
      return out;
    }

    std::ostream& operator<<(std::ostream& out,
                             const MouseEvent::Button::Id& id) {
      switch (id) {
        case MouseEvent::Button::Left:
          return out << "Left";
        case MouseEvent::Button::Middle:
          return out << "Middle";
        case MouseEvent::Button::Button2:
          return out << "Button2";
        case MouseEvent::Button::Button3:
          return out << "Button3";
        case MouseEvent::Button::Button5:
          return out << "Button5";
        case MouseEvent::Button::Button6:
          return out << "Button6";
        case MouseEvent::Button::Button7:
          return out << "Button7";
        case MouseEvent::Button::Button8:
          return out << "Button8";
        default:
          out << "None";
      }
    }

    std::ostream& operator<<(std::ostream& out,
                             const MouseEvent::Button& button) {
      return out << button.button << "["
                 << (button.wasPressed ? "PRESSED" : "RELEASED") << " -> "
                 << (button.pressed ? "PRESSED" : "RELEASED") << "]";
    }

    std::ostream& operator<<(std::ostream& out, const MouseEvent& event) {
      return out << "@" << event.position << ": " << event.button << ": "
                 << event.modifiers;
    }

    class GLCanvas {
      private:
      unsigned int width, height;
      Region2D<float> region;
      MouseEvent lastMouseEvent;

#ifdef __EMSCRIPTEN__
      std::string id;
      EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
#else
      GLFWwindow* window = nullptr;
#endif

      std::vector<std::function<void(GLCanvas&, int, int)>> onResize;
      std::vector<std::function<void(GLCanvas&, const MouseEvent&)>>
        onMouseEvent;

      void resizeViewport(float width, float height) {
        glViewport(0, 0, width, height);
        region.min = {0, 0};
        region.max = {width, height};

#ifdef __EMSCRIPTEN__
        EM_ASM_(
          {
            const id = Module.Pointer_stringify($0);
            const canvas = document.getElementById(id);
            canvas.width = $1;
            canvas.height = $2;
          },
          id.c_str(), width, height);
#endif
      }

      public:
      GLCanvas(int width, int height, const char* title = "empirical")
        : width(width),
          height(height)
#ifdef __EMSCRIPTEN__
          ,
          id(title)
#endif
      {
#ifdef __EMSCRIPTEN__
        EmscriptenWebGLContextAttributes attrs;
        emscripten_webgl_init_context_attributes(&attrs);
        attrs.majorVersion = 2;
        attrs.minorVersion = 0;
        context = emscripten_webgl_create_context(title, &attrs);

#else
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 8);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(
          window, [](GLFWwindow* window, int width, int height) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));

            me->width = width;
            me->height = height;
            me->resizeViewport(width, height);

            me->onResize.erase(
              std::remove_if(me->onResize.begin(), me->onResize.end(),
                             [](auto& fn) { return !fn; }),
              me->onResize.end());
            for (auto& callback : me->onResize) callback(*me, width, height);
          });

        glfwSetCursorPosCallback(
          window, [](GLFWwindow* window, double xpos, double ypos) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));

            me->lastMouseEvent.position.x() = xpos;
            me->lastMouseEvent.position.y() = ypos;

            me->onMouseEvent.erase(
              std::remove_if(me->onMouseEvent.begin(), me->onMouseEvent.end(),
                             [](auto& fn) { return !fn; }),
              me->onMouseEvent.end());
            for (auto& callback : me->onMouseEvent)
              callback(*me, me->lastMouseEvent);
          });

        glfwSetMouseButtonCallback(
          window, [](GLFWwindow* window, int b, int action, int mods) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));
            auto button{static_cast<MouseEvent::Button::Id>(b)};

            std::cout << b << std::endl;
            me->lastMouseEvent.button.wasPressed =
              me->lastMouseEvent.button.pressed &&
              me->lastMouseEvent.button.button == button;

            me->lastMouseEvent.button.button = button;
            me->lastMouseEvent.button.pressed = (action == GLFW_PRESS);

            me->lastMouseEvent.modifiers =
              static_cast<MouseEvent::Modifiers>(mods);

            me->onMouseEvent.erase(
              std::remove_if(me->onMouseEvent.begin(), me->onMouseEvent.end(),
                             [](auto& fn) { return !fn; }),
              me->onMouseEvent.end());
            for (auto& callback : me->onMouseEvent)
              callback(*me, me->lastMouseEvent);
            me->lastMouseEvent.button.wasPressed =
              me->lastMouseEvent.button.pressed;
          });
#endif
        makeCurrent();
        resizeViewport(width, height);

#if !defined(EMSCRIPTEN)
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
          throw std::runtime_error("Could not initialize GLEW");
        }
#endif  // !defined(EMSCRIPTEN)
      }

#ifdef __EMSCRIPTEN__
      GLCanvas(const char* title = "empirical")
        : GLCanvas(getWindowWidth(), getWindowHeight(), title) {}
#else
      GLCanvas(const char* title = "empirical") : GLCanvas(800, 600, title) {}
#endif
      GLCanvas(const GLCanvas&) = delete;
      GLCanvas(GLCanvas&& other) = delete;
      GLCanvas& operator=(const GLCanvas&) = delete;
      GLCanvas& operator=(GLCanvas&& other) = delete;

      ~GLCanvas() {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#else
        if (window != nullptr) {
          glfwDestroyWindow(window);
        }
        glfwTerminate();
#endif
      }

      template <typename F>
      void bindOnResize(F&& callback) {
        onResize.emplace_back(std::forward<F>(callback));
      }

      template <typename F>
      void bindOnMouseEvent(F&& callback) {
        onMouseEvent.emplace_back(std::forward<F>(callback));
      }

      template <typename R>
      void runForever(R&& onUpdate, int fps = -1, bool forever = true) {
        makeCurrent();
#ifdef __EMSCRIPTEN__
        auto args = std::make_tuple(std::forward<R>(onUpdate), this);
        emscripten_set_main_loop_arg(
          [](void* argsUnsafe) {
            auto a = reinterpret_cast<decltype(args)*>(argsUnsafe);
            std::get<0> (*a)(*std::get<1>(*a));
          },
          &args, fps, forever);
#else
        if (fps <= 0) fps = 60;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
        auto frameStart = std::chrono::system_clock::now();

        while (!glfwWindowShouldClose(window)) {
          auto frameCurrent = std::chrono::system_clock::now();
          if (frameCurrent - frameStart >=
              std::chrono::milliseconds((int)(1000.f / fps))) {
            onUpdate(*this);
            glfwSwapBuffers(window);
            frameStart = std::chrono::system_clock::now();
          }
          glfwPollEvents();
        }
#endif
      }

      void makeCurrent() {
#ifdef __EMSCRIPTEN__
        emscripten_webgl_make_context_current(context);
#else
        glfwMakeContextCurrent(window);
#endif
      }

      VertexArrayObject makeVAO() { return VertexArrayObject(); }
      template <typename... Args>
      ShaderProgram makeShaderProgram(Args&&... args) const {
        return {std::forward<Args>(args)...};
      }

      template <BufferType TYPE>
      BufferObject<TYPE> makeBuffer() {
        GLuint handle;
        glGenBuffers(1, &handle);
        utils::catchGlError();
        return BufferObject<TYPE>(handle);
      }

      auto getWidth() const { return width; }
      auto getHeight() const { return height; }

      auto getRegion() const { return region; }
    };

  }  // namespace opengl
}  // namespace emp

#endif
