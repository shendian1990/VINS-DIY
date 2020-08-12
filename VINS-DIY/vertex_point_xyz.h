#pragma once

#include "vertex.h"

namespace myslam {
    namespace backend {

        /**
         * @brief ��xyz��ʽ�������Ķ���
         */
        class VertexPointXYZ : public Vertex {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            VertexPointXYZ() : Vertex(3) {}

            std::string TypeInfo() const { return "VertexPointXYZ"; }
        };

    }
}

