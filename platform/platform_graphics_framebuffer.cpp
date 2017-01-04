//
// Created by hogsy on 03/01/17.
//

#include "platform_graphics_framebuffer.h"

namespace pl {
    namespace graphics {

        class RenderBuffer {
        public:
            RenderBuffer(unsigned int width = 512, unsigned int height = 512);
            ~RenderBuffer();

        protected:
        private:
        };

        class FrameBuffer {
        public:
            FrameBuffer(unsigned int width = 512, unsigned int height = 512);
            ~FrameBuffer();

        protected:
        private:
        };

        class PostProcess {
        public:
        protected:
        private:
        };

    }
}