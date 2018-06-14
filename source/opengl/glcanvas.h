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

#include <algorithm>
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
        enum Id :
#ifdef __EMSCRIPTEN__
          unsigned short
#else
          int
#endif
        {
#ifdef __EMSCRIPTEN__
          Left = 0,
          Middle = 1,
          Right = 2
#else
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
#endif
        } button;
        template <typename B>
        void press(bool pressed, B button) {
          wasPressed = this->pressed;
          this->pressed = pressed;
          this->button = static_cast<Id>(button);
        }

        bool Clicked() const { return wasPressed && !pressed; }

        bool wasPressed;
        bool pressed;
      };

      class Modifiers {
        private:
#ifdef __EMSCRIPTEN__
        bool shift_set;
        bool control_set;
        bool alt_set;
        bool super_set;
#else
        int mod;
#endif
        public:
#ifdef __EMSCRIPTEN__
        Modifiers(bool shift = false, bool control = false, bool alt = false,
                  bool super = false)
          : shift_set(shift),
            control_set(control),
            alt_set(alt),
            super_set(super) {}
#else
        Modifiers(int mod = 0) : mod(mod) {}
#endif
        Modifiers(const Modifiers&) = default;
        Modifiers(Modifiers&&) = default;
        Modifiers& operator=(const Modifiers&) = default;
        Modifiers& operator=(Modifiers&&) = default;

#ifdef __EMSCRIPTEN__
        void set_shift(bool value) { shift_set = value; }
        void set_control(bool value) { control_set = value; }
        void set_alt(bool value) { alt_set = value; }
        void set_super(bool value) { super_set = value; }

        bool shift() const { return shift_set; }
        bool control() const { return control_set; }
        bool alt() const { return alt_set; }
        bool super() const { return super_set; }
#else
        bool shift() const { return (mod & GLFW_MOD_SHIFT) != 0; }
        bool control() const { return (mod & GLFW_MOD_CONTROL) != 0; }
        bool alt() const { return (mod & GLFW_MOD_ALT) != 0; }
        bool super() const { return (mod & GLFW_MOD_SUPER) != 0; }

        void reset() { mod = 0; }
#endif
      };

      Vec2d position;
      Button button;
      Modifiers modifiers;

      MouseEvent() : position(0, 0), button{Button::Left, false, false} {}
      MouseEvent(double x, double y, const Button& button,
                 const Modifiers& modifiers = {})
        : position(x, y), button(button), modifiers(modifiers) {}
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
        case MouseEvent::Button::Right:
          return out << "Right";
#ifndef __EMSCRIPTEN__
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
#endif
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

    template <typename Fn>
    class Event {
      private:
      using fn_t = std::function<Fn>;
      std::vector<fn_t> listeners;

      public:
      Event() {}
      Event(const Event&) = delete;
      Event(Event&&) = delete;
      Event& operator=(const Event&) = delete;
      Event& operator=(Event&&) = delete;

      template <typename... U>
      void fire(U&&... args) {
        if (listeners.empty()) return;

        for (size_t i = 0; i < listeners.size() - 1; ++i) {
          listeners[i](args...);
        }
        listeners.back()(std::forward<U>(args)...);
      }

      template <typename... U>
      void operator()(U&&... args) {
        fire(std::forward<U>(args)...);
      }

      template <typename L>
      void bind(L listener) {
        listeners.push_back(listener);
      }

      template <typename L>
      bool unbind(const L& listener) {
        auto location =
          std::find_if(listeners.begin(), listeners.end(),
                       [listener = std::forward<L>(listener)](
                         const auto& other) { return other == listener; });
        if (location != listeners.end()) {
          listeners.erase(location);
          return true;
        }
        return false;
      }
    };

    template <typename Fn>
    class EventHandle {
      private:
      Event<Fn>* event;

      public:
      EventHandle(Event<Fn>* event) : event(event) {}
      EventHandle(const EventHandle&) = delete;
      EventHandle(EventHandle&&) = delete;
      EventHandle& operator=(const EventHandle&) = delete;
      EventHandle& operator=(EventHandle&&) = delete;

      template <typename L>
      void bind(L&& listener) {
        event->bind(std::forward<L>(listener));
      }
      template <typename L>
      bool unbind(L&& listener) {
        return event->unbind(std::forward<L>(listener));
      }
    };

#define ADD_EVENT(name, sig) \
  private:                   \
  Event<sig> name##_event{}; \
                             \
  public:                    \
  EventHandle<sig> on_##name##_event{&name##_event};

    class GLCanvas {
      ADD_EVENT(mouse, void(GLCanvas&, const MouseEvent&))
      ADD_EVENT(resize, void(GLCanvas&, int, int))

      private:
      int width, height;
      Region2f region;
      MouseEvent lastMouseEvent;

#ifdef __EMSCRIPTEN__
      std::string id;
      EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
#else
      GLFWwindow* window = nullptr;
#endif

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
        attrs.majorVersion = 1;
        attrs.minorVersion = 0;
        context = emscripten_webgl_create_context(title, &attrs);

        emscripten_set_resize_callback(
          0, this, false,
          [](int, const EmscriptenUiEvent* event, void* data_ptr) -> int {
            auto& me = *static_cast<GLCanvas*>(data_ptr);
            int width = EM_ASM_INT(
              {
                const id = Module.Pointer_stringify($0);
                const canvas = document.getElementById(id);
                return canvas.offsetWidth;
              },
              me.id.c_str());
            int height = EM_ASM_INT(
              {
                const id = Module.Pointer_stringify($0);
                const canvas = document.getElementById(id);
                return canvas.offsetHeight;
              },
              me.id.c_str());

            me.resize_event.fire(me, width, height);

            return false;
          });
        emscripten_set_mousedown_callback(
          title, this, false,
          [](int, const EmscriptenMouseEvent* event, void* data_ptr) -> int {
            auto& me = *static_cast<GLCanvas*>(data_ptr);
            me.lastMouseEvent.position.x() = event->clientX;
            me.lastMouseEvent.position.y() = event->clientY;

            me.lastMouseEvent.modifiers.set_shift(event->shiftKey);
            me.lastMouseEvent.modifiers.set_control(event->ctrlKey);
            me.lastMouseEvent.modifiers.set_alt(event->altKey);
            me.lastMouseEvent.modifiers.set_super(event->metaKey);

            me.lastMouseEvent.button.press(true, event->button);
            me.mouse_event.fire(me, me.lastMouseEvent);
            return false;
          });

        emscripten_set_mouseup_callback(
          title, this, false,
          [](int, const EmscriptenMouseEvent* event, void* data_ptr) -> int {
            auto& me = *static_cast<GLCanvas*>(data_ptr);
            me.lastMouseEvent.position.x() = event->clientX;
            me.lastMouseEvent.position.y() = event->clientY;

            me.lastMouseEvent.modifiers.set_shift(event->shiftKey);
            me.lastMouseEvent.modifiers.set_control(event->ctrlKey);
            me.lastMouseEvent.modifiers.set_alt(event->altKey);
            me.lastMouseEvent.modifiers.set_super(event->metaKey);

            me.lastMouseEvent.button.press(false, event->button);
            me.mouse_event.fire(me, me.lastMouseEvent);
            return false;
          });

        emscripten_set_mousemove_callback(
          title, this, false,
          [](int, const EmscriptenMouseEvent* event, void* data_ptr) -> int {
            auto& me = *static_cast<GLCanvas*>(data_ptr);
            me.lastMouseEvent.position.x() = event->clientX;
            me.lastMouseEvent.position.y() = event->clientY;

            me.lastMouseEvent.modifiers.set_shift(event->shiftKey);
            me.lastMouseEvent.modifiers.set_control(event->ctrlKey);
            me.lastMouseEvent.modifiers.set_alt(event->altKey);
            me.lastMouseEvent.modifiers.set_super(event->metaKey);

            me.mouse_event.fire(me, me.lastMouseEvent);
            return false;
          });
#else
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 16);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(
          window, [](GLFWwindow* window, int width, int height) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));

            me->width = width;
            me->height = height;
            me->resizeViewport(width, height);

            me->resize_event.fire(*me, width, height);
          });

        glfwSetCursorPosCallback(
          window, [](GLFWwindow* window, double xpos, double ypos) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));

            me->lastMouseEvent.position.x() = xpos;
            me->lastMouseEvent.position.y() = ypos;
            me->lastMouseEvent.modifiers.reset();

            me->mouse_event.fire(*me, me->lastMouseEvent);
          });

        glfwSetMouseButtonCallback(
          window, [](GLFWwindow* window, int b, int action, int mods) {
            GLCanvas* me =
              static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));
            auto button{static_cast<MouseEvent::Button::Id>(b)};

            me->lastMouseEvent.button.wasPressed =
              me->lastMouseEvent.button.pressed &&
              me->lastMouseEvent.button.button == button;

            me->lastMouseEvent.button.button = button;
            me->lastMouseEvent.button.pressed = (action == GLFW_PRESS);

            me->lastMouseEvent.modifiers =
              static_cast<MouseEvent::Modifiers>(mods);

            me->mouse_event.fire(*me, me->lastMouseEvent);

            me->lastMouseEvent.button.wasPressed =
              me->lastMouseEvent.button.pressed;
          });
#endif
        makeCurrent();
        resizeViewport(width, height);

#if !defined(EMSCRIPTEN)
        glewExperimental = GL_TRUE;
#if !defined(EMP_NDEBUG)
        auto glew_status =
#endif
          glewInit();

        emp_assert(glew_status == GLEW_OK, "GLEW initialized");

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

      template <typename R>
      void runForever(R&& onUpdate, int fps = -1, bool forever = true) {
        makeCurrent();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

      VertexArrayObject MakeVAO() { return VertexArrayObject(); }
      template <typename... Args>
      ShaderProgram makeShaderProgram(Args&&... args) const {
        return {std::forward<Args>(args)...};
      }

      template <BufferType TYPE>
      BufferObject<TYPE> makeBuffer() {
        GLuint handle;
        emp_checked_gl_void(glGenBuffers(1, &handle));
        return BufferObject<TYPE>(handle);
      }

      auto getWidth() const { return width; }
      auto getHeight() const { return height; }

      auto getRegion() const { return region; }
    };

  }  // namespace opengl
}  // namespace emp

#endif
