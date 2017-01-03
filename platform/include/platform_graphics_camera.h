
#pragma once

namespace pl {
    namespace graphics {
        
        class Viewport;

        class Camera {
        public:
            Camera();
            Camera(Viewport *viewport);

            virtual void Draw();
            virtual void Simulate();

            // todo, input

            void SetAngles(float x, float y, float z);
            void SetAngles(math::Vector3D angles);
            void SetPitch(float pitch);
            void PrintAngles();

        protected:
        private:
            math::Vector3D angles_, position_;

            float fov_, fovx_, fovy_;
        };

    }
}