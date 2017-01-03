
#pragma once

namespace pl {
    namespace graphics {

        class Camera;

        class PL_DLL Viewport {
        public:
            Viewport(int x = 0, int y = 0, unsigned int width = 640, unsigned int height = 480);

            math::Vector2D GetSize() { return math::Vector2D(width_, height_); }
            unsigned int GetWidth() { return width_; }
            unsigned int GetHeight() { return height_; }

            void SetSize(unsigned int width, unsigned int height);

            math::Vector2D GetPosition() { return math::Vector2D(x_, y_); }

            void SetPosition(int x, int y);

            virtual void Draw() = 0;

            void Screenshot(std::string path);
            void Screenshot();

        protected:
        private:
            int x_, y_;
            unsigned int width_, height_;

            Camera *camera_;

            std::vector<Viewport*> children_;
            Viewport*parent_;
        };

    }
}