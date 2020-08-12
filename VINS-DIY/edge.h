#pragma once

#include <memory>
#include <string>
#include "eigen_types.h"
#include <eigen3/Eigen/Dense>
#include "loss_function.h"

namespace myslam {
    namespace backend {

        class Vertex;

        /**
         * �߸������в�в��� Ԥ��-�۲⣬ά���ڹ��캯���ж���
         * ���ۺ����� �в�*��Ϣ*�в��һ����ֵ���ɺ����ͺ���С��
         */
        class Edge {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            /**
             * ���캯�������Զ������ſɱȵĿռ�
             * @param residual_dimension �в�ά��
             * @param num_verticies ��������
             * @param verticies_types �����������ƣ����Բ����������Ļ�check�в�����
             */
            explicit Edge(int residual_dimension, int num_verticies,
                const std::vector<std::string>& verticies_types = std::vector<std::string>());

            virtual ~Edge();

            /// ����id
            unsigned long Id() const { return id_; }

            /**
             * ����һ������
             * @param vertex ��Ӧ��vertex����
             */
            bool AddVertex(std::shared_ptr<Vertex> vertex) {
                verticies_.emplace_back(vertex);
                return true;
            }

            /**
             * ����һЩ����
             * @param vertices ���㣬������˳������
             * @return
             */
            bool SetVertex(const std::vector<std::shared_ptr<Vertex>>& vertices) {
                verticies_ = vertices;
                return true;
            }

            /// ���ص�i������
            std::shared_ptr<Vertex> GetVertex(int i) {
                return verticies_[i];
            }

            /// �������ж���
            std::vector<std::shared_ptr<Vertex>> Verticies() const {
                return verticies_;
            }

            /// ���ع����������
            size_t NumVertices() const { return verticies_.size(); }

            /// ���رߵ�������Ϣ����������ʵ��
            virtual std::string TypeInfo() const = 0;

            /// ����в������ʵ��
            virtual void ComputeResidual() = 0;

            /// �����ſɱȣ�������ʵ��
            /// ����˲�֧���Զ��󵼣���Ҫʵ��ÿ��������ſɱȼ��㷽��
            virtual void ComputeJacobians() = 0;

            //    ///�����edge��Hession�����Ӱ�죬������ʵ��
            //    virtual void ComputeHessionFactor() = 0;

                /// ����ƽ�����������Ϣ����
            double Chi2() const;
            double RobustChi2() const;

            /// ���زв�
            VecX Residual() const { return residual_; }

            /// �����ſɱ�
            std::vector<MatXX> Jacobians() const { return jacobians_; }

            /// ������Ϣ����
            void SetInformation(const MatXX& information) {
                information_ = information;
                // sqrt information
                sqrt_information_ = Eigen::LLT<MatXX>(information_).matrixL().transpose();
            }

            /// ������Ϣ����
            MatXX Information() const {
                return information_;
            }

            MatXX SqrtInformation() const {
                return sqrt_information_;
            }

            void SetLossFunction(LossFunction* ptr) { lossfunction_ = ptr; }
            LossFunction* GetLossFunction() { return lossfunction_; }
            void RobustInfo(double& drho, MatXX& info) const;

            /// ���ù۲���Ϣ
            void SetObservation(const VecX& observation) {
                observation_ = observation;
            }

            /// ���ع۲���Ϣ
            VecX Observation() const { return observation_; }

            /// ���ߵ���Ϣ�Ƿ�ȫ������
            bool CheckValid();

            int OrderingId() const { return ordering_id_; }

            void SetOrderingId(int id) { ordering_id_ = id; };

        protected:
            unsigned long id_;  // edge id
            int ordering_id_;   //edge id in problem
            std::vector<std::string> verticies_types_;  // ������������Ϣ������debug
            std::vector<std::shared_ptr<Vertex>> verticies_; // �ñ߶�Ӧ�Ķ���
            VecX residual_;                 // �в�
            std::vector<MatXX> jacobians_;  // �ſɱȣ�ÿ���ſɱ�ά���� residual x vertex[i]
            MatXX information_;             // ��Ϣ����
            MatXX sqrt_information_;
            VecX observation_;              // �۲���Ϣ

            LossFunction* lossfunction_;
        };

    }
}
