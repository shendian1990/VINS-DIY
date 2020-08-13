#include <iostream>
#include <random>
#include "vertex_inverse_depth.h"
#include "vertex_pose.h"
#include "edge_reprojection.h"
#include "problem.h"

using namespace myslam::backend;
using namespace std;

/*
 * Frame : ����ÿ֡����̬�͹۲�
 */
struct Frame {
    Frame(Eigen::Matrix3d R, Eigen::Vector3d t) : Rwc(R), qwc(R), twc(t) {};
    Eigen::Matrix3d Rwc;
    Eigen::Quaterniond qwc;
    Eigen::Vector3d twc;

    unordered_map<int, Eigen::Vector3d> featurePerId; // ��֡�۲⵽�������Լ�����id
};

/*
 * ������������ϵ�µ���������: �����̬, ������, �Լ�ÿ֡�۲�
 */
void GetSimDataInWordFrame(vector<Frame>& cameraPoses, vector<Eigen::Vector3d>& points) {
    int featureNums = 20;  // ������Ŀ������ÿ֡���ܹ۲⵽���е�����
    int poseNums = 3;     // �����Ŀ

    double radius = 8;
    for (int n = 0; n < poseNums; ++n) {
        double theta = n * 2 * M_PI / (poseNums * 4); // 1/4 Բ��
        // �� z�� ��ת
        Eigen::Matrix3d R;
        R = Eigen::AngleAxisd(theta, Eigen::Vector3d::UnitZ());
        Eigen::Vector3d t = Eigen::Vector3d(radius * cos(theta) - radius, radius * sin(theta), 1 * sin(2 * theta));
        cameraPoses.push_back(Frame(R, t));
    }

    // �����������ά������
    std::default_random_engine generator;
    std::normal_distribution<double> noise_pdf(0., 1. / 1000.);  // 2pixel / focal
    for (int j = 0; j < featureNums; ++j) {
        std::uniform_real_distribution<double> xy_rand(-4, 4.0);
        std::uniform_real_distribution<double> z_rand(4., 8.);

        Eigen::Vector3d Pw(xy_rand(generator), xy_rand(generator), z_rand(generator));
        points.push_back(Pw);

        // ��ÿһ֡�ϵĹ۲���
        for (int i = 0; i < poseNums; ++i) {
            Eigen::Vector3d Pc = cameraPoses[i].Rwc.transpose() * (Pw - cameraPoses[i].twc);
            Pc = Pc / Pc.z();  // ��һ��ͼ��ƽ��
            Pc[0] += noise_pdf(generator);
            Pc[1] += noise_pdf(generator);
            cameraPoses[i].featurePerId.insert(make_pair(j, Pc));
        }
    }
}
/*
int main() {
    // ׼������
    vector<Frame> cameras;
    vector<Eigen::Vector3d> points;
    GetSimDataInWordFrame(cameras, points);
    Eigen::Quaterniond qic(1, 0, 0, 0);
    Eigen::Vector3d tic(0, 0, 0);

    // ���� problem
    Problem problem(Problem::ProblemType::SLAM_PROBLEM);

    // ���� Pose
    vector<shared_ptr<VertexPose> > vertexCams_vec;
    for (size_t i = 0; i < cameras.size(); ++i) {
        shared_ptr<VertexPose> vertexCam(new VertexPose());
        Eigen::VectorXd pose(7);
        pose << cameras[i].twc, cameras[i].qwc.x(), cameras[i].qwc.y(), cameras[i].qwc.z(), cameras[i].qwc.w(); //ƽ�ƺ���Ԫ��
        vertexCam->SetParameters(pose);

        //        if(i < 2)
        //            vertexCam->SetFixed();

        problem.AddVertex(vertexCam);
        vertexCams_vec.push_back(vertexCam);
        vertexCams_vec.push_back(vertexCam);
    }

    // ���� Point �� edge
    std::default_random_engine generator;
    std::normal_distribution<double> noise_pdf(0, 1.);
    double noise = 0;
    vector<double> noise_invd;
    vector<shared_ptr<VertexInverseDepth> > allPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        //�����������������ʼ֡Ϊ��0֡�� ��������׵õ�
        Eigen::Vector3d Pw = points[i];
        Eigen::Vector3d Pc = cameras[0].Rwc.transpose() * (Pw - cameras[0].twc);
        noise = noise_pdf(generator);
        double inverse_depth = 1. / (Pc.z() + noise);
        //        double inverse_depth = 1. / Pc.z();
        noise_invd.push_back(inverse_depth);

        // ��ʼ������ vertex
        shared_ptr<VertexInverseDepth> verterxPoint(new VertexInverseDepth());
        VecX inv_d(1);
        inv_d << inverse_depth;
        verterxPoint->SetParameters(inv_d);
        problem.AddVertex(verterxPoint);
        allPoints.push_back(verterxPoint);

        // ÿ��������Ӧ��ͶӰ���, �� 0 ֡Ϊ��ʼ֡
        for (size_t j = 1; j < cameras.size(); ++j) {
            Eigen::Vector3d pt_i = cameras[0].featurePerId.find(i)->second;
            Eigen::Vector3d pt_j = cameras[j].featurePerId.find(i)->second;
            shared_ptr<EdgeReprojection> edge(new EdgeReprojection(pt_i, pt_j));
            edge->SetTranslationImuFromCamera(qic, tic);

            std::vector<std::shared_ptr<Vertex> > edge_vertex;
            edge_vertex.push_back(verterxPoint);
            edge_vertex.push_back(vertexCams_vec[0]);
            edge_vertex.push_back(vertexCams_vec[j]);
            edge->SetVertex(edge_vertex);

            problem.AddEdge(edge);
        }
    }

    problem.Solve(5); //һ������5��

    std::cout << "\nCompare MonoBA results after opt..." << std::endl;
    for (size_t k = 0; k < allPoints.size(); k += 1) {
        std::cout << "after opt, point " << k << " : gt " << 1. / points[k].z() << " ,noise "
            << noise_invd[k] << " ,opt " << allPoints[k]->Parameters() << std::endl;
    }
    std::cout << "------------ pose translation ----------------" << std::endl;
    for (int i = 0; i < vertexCams_vec.size(); ++i) {
        std::cout << "translation after opt: " << i << " :" << vertexCams_vec[i]->Parameters().head(3).transpose() << " || gt: " << cameras[i].twc.transpose() << std::endl;
    }
    /// �Ż���ɺ󣬵�һ֡����� pose ƽ�ƣ�x,y,z��������ԭ�� 0,0,0. ˵������ռ䷢����Ư�ơ�
    /// ����취�� fix ��һ֡�͵ڶ�֡���̶� 7 ���ɶȡ� ���߼��Ϸǳ��������ֵ��

    problem.TestMarginalize();

    return 0;
}
*/
