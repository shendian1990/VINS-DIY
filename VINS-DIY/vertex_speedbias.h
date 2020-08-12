#pragma once
#include <memory>
#include "vertex.h"

namespace myslam {
    namespace backend {

        /**
         * SpeedBias vertex
         * parameters: v, ba, bg 9 DoF
         *
         */
        class VertexSpeedBias : public Vertex {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            VertexSpeedBias() : Vertex(9) {}

            std::string TypeInfo() const {
                return "VertexSpeedBias";
            }

        };

    }
}
