#pragma once
//
// Created by heyijia on 19-1-30.
//

#include <memory>
#include <string>

#include <Eigen/Dense>

#include "eigen_types.h"
#include "edge.h"


namespace myslam {
    namespace backend {

        /**
        * EdgeSE3Prior���˱�Ϊ 1 Ԫ�ߣ���֮�����Ķ����У�Ti
        */
        class EdgeSE3Prior : public Edge {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            EdgeSE3Prior(const Vec3& p, const Qd& q) :
                Edge(6, 1, std::vector<std::string>{"VertexPose"}),
                Pp_(p), Qp_(q) {}

            /// ���رߵ�������Ϣ
            virtual std::string TypeInfo() const override { return "EdgeSE3Prior"; }

            /// ����в�
            virtual void ComputeResidual() override;

            /// �����ſɱ�
            virtual void ComputeJacobians() override;


        private:
            Vec3 Pp_;   // pose prior
            Qd   Qp_;   // Rotation prior
        };

    }
}
