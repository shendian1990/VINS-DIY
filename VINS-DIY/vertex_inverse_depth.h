#pragma once

#include "vertex.h"

namespace myslam {
    namespace backend {

        /**
         * ���������ʽ�洢�Ķ���
         */
        class VertexInverseDepth : public Vertex {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            VertexInverseDepth() : Vertex(1) {}

            virtual std::string TypeInfo() const { return "VertexInverseDepth"; }
        };

    }
}
