#pragma once

#include <memory>
#include "vertex.h"

namespace myslam {
    namespace backend {

        /**
         * Pose vertex
         * parameters: tx, ty, tz, qx, qy, qz, qw, 7 DoF
         * optimization is perform on manifold, so update is 6 DoF, left multiplication
         *
         * pose is represented as Twb in VIO case
         */
        class VertexPose : public Vertex {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            VertexPose() : Vertex(7, 6) {}

            /// �ӷ������ض���
            /// Ĭ����������
            virtual void Plus(const VecX& delta) override;

            std::string TypeInfo() const {
                return "VertexPose";
            }

            /**
             * ��Ҫά��[H|b]�����е��������ݿ�
             * p: pose, m:mappoint
             *
             *     Hp1_p2
             *     Hp2_p2    Hp2_m1    Hp2_m2    Hp2_m3     |    bp2
             *
             *                         Hm2_m2               |    bm2
             *                                   Hm2_m3     |    bm3
             * 1. ����CameraΪsource camera����ά��vHessionSourceCamera��
             * 2. ����CameraΪmeasurement camera, ��ά��vHessionMeasurementCamera��
             * 3. ��һֱά��m_HessionDiagonal��
             */
        };

    }
}
