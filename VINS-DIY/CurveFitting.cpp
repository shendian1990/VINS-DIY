/*#include <iostream>
#include <random>
#include "vertex_pose.h"
#include "edge_reprojection.h"
#include "problem.h"

using namespace myslam::backend;
using namespace std;

// ����ģ�͵Ķ��㣬ģ��������Ż�����ά�Ⱥ���������
class CurveFittingVertex : public Vertex
{
public:
    //EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    CurveFittingVertex() : Vertex(3) {}
    virtual std::string TypeInfo() const { return "abc"; }
};

// ���ģ�� ģ��������۲�ֵά�ȣ����ͣ����Ӷ�������
class CurveFittingEdge : public Edge
{
public:
    //EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    CurveFittingEdge(double x, double y) : Edge(1, 1, std::vector<std::string>{"abc"}) {
        x_ = x;
        y_ = y;
    }
    // ��������ģ�����
    virtual void ComputeResidual() override
    {
        Vec3 abc = verticies_[0]->Parameters();
        residual_(0) = std::exp(abc(0) * x_ * x_ + abc(1) * x_ + abc(2)) - y_;
    }
    virtual void ComputeJacobians() override
    {
        Vec3 abc = verticies_[0]->Parameters();
        double exp_y = std::exp(abc(0) * x_ * x_ + abc(1) * x_ + abc(2));
        Eigen::Matrix<double, 1, 3> jaco_abc;
        jaco_abc << x_ * x_ * exp_y, x_* exp_y, 1 * exp_y;
        jacobians_[0] = jaco_abc;
    }
    /// ���رߵ�������Ϣ
    virtual std::string TypeInfo() const override { return "CurveFittingEdge"; }
public:
    double x_, y_;  // x ֵ�� y ֵΪ _measurement
};

int main()
{
    double a = 1.0, b = 2.0, c = 1.0;         // ��ʵ����ֵ
    int N = 100;                          // ���ݵ�
    double w_sigma = 1.;                 // ����Sigmaֵ

    std::default_random_engine generator;
    std::normal_distribution<double> noise(0., w_sigma);

    // ���� problem
    Problem problem(Problem::ProblemType::GENERIC_PROBLEM);
    shared_ptr< CurveFittingVertex > vertex(new CurveFittingVertex());
    vertex->SetParameters(Eigen::Vector3d(0., 0., 0.));

    problem.AddVertex(vertex);
    for (int i = 0; i < N; ++i) {
        double x = i / 100.;
        double n = noise(generator);
        double y = std::exp(a * x * x + b * x + c) + n;
        //        double y = std::exp( a*x*x + b*x + c );
        shared_ptr< CurveFittingEdge > edge(new CurveFittingEdge(x, y));
        std::vector<std::shared_ptr<Vertex>> edge_vertex;
        edge_vertex.push_back(vertex);
        edge->SetVertex(edge_vertex);

        problem.AddEdge(edge);
    }

    std::cout << "\nTest CurveFitting start..." << std::endl;
    problem.Solve(30);
    std::cout << vertex->Parameters() << std::endl;
    return 0;
}
*/
