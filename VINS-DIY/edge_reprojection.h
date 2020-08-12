#pragma once

#include <memory>
#include <string>

#include <Eigen/Dense>

#include "eigen_types.h"
#include "edge.h"

namespace myslam {
    namespace backend {

        /**
         * �˱����Ӿ���ͶӰ���˱�Ϊ��Ԫ�ߣ���֮�����Ķ����У�
         * ·���������InveseDepth����һ�ι۲⵽��·����source Camera��λ��T_World_From_Body1��
         * �͹۲⵽��·����mearsurement Cameraλ��T_World_From_Body2��
         * ע�⣺verticies_����˳�����ΪInveseDepth��T_World_From_Body1��T_World_From_Body2��
         */
        class EdgeReprojection : public Edge {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            EdgeReprojection(const Vec3& pts_i, const Vec3& pts_j)
                : Edge(2, 4, std::vector<std::string>{"VertexInverseDepth", "VertexPose", "VertexPose", "VertexPose"}) {
                pts_i_ = pts_i;
                pts_j_ = pts_j;
            }

            /// ���رߵ�������Ϣ
            virtual std::string TypeInfo() const override { return "EdgeReprojection"; }

            /// ����в�
            virtual void ComputeResidual() override;

            /// �����ſɱ�
            virtual void ComputeJacobians() override;

            //    void SetTranslationImuFromCamera(Eigen::Quaterniond &qic_, Vec3 &tic_);

        private:
            //Translation imu from camera
        //    Qd qic;
        //    Vec3 tic;

            //measurements
            Vec3 pts_i_, pts_j_;
        };

        /**
        * �˱����Ӿ���ͶӰ���˱�Ϊ��Ԫ�ߣ���֮�����Ķ����У�
        * ·������������ϵXYZ���۲⵽��·���� Camera ��λ��T_World_From_Body1
        * ע�⣺verticies_����˳�����Ϊ XYZ��T_World_From_Body1��
        */
        class EdgeReprojectionXYZ : public Edge {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            EdgeReprojectionXYZ(const Vec3& pts_i)
                : Edge(2, 2, std::vector<std::string>{"VertexXYZ", "VertexPose"}) {
                obs_ = pts_i;
            }

            /// ���رߵ�������Ϣ
            virtual std::string TypeInfo() const override { return "EdgeReprojectionXYZ"; }

            /// ����в�
            virtual void ComputeResidual() override;

            /// �����ſɱ�
            virtual void ComputeJacobians() override;

            void SetTranslationImuFromCamera(Eigen::Quaterniond& qic_, Vec3& tic_);

        private:
            //Translation imu from camera
            Qd qic;
            Vec3 tic;

            //measurements
            Vec3 obs_;
        };

        /**
         * ��������ͶӰpose������
         */
        class EdgeReprojectionPoseOnly : public Edge {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            EdgeReprojectionPoseOnly(const Vec3& landmark_world, const Mat33& K) :
                Edge(2, 1, std::vector<std::string>{"VertexPose"}),
                landmark_world_(landmark_world), K_(K) {}

            /// ���رߵ�������Ϣ
            virtual std::string TypeInfo() const override { return "EdgeReprojectionPoseOnly"; }

            /// ����в�
            virtual void ComputeResidual() override;

            /// �����ſɱ�
            virtual void ComputeJacobians() override;

        private:
            Vec3 landmark_world_;
            Mat33 K_;
        };

    }
}

