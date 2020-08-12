#include "feature_tracker.h"

#include "parameters.h"
#include "CameraFactory.h"
#include "PinholeCamera.h"

//FeatureTracker��static��Ա����n_id��ʼ��Ϊ0
int FeatureTracker::n_id = 0;

//�жϸ��ٵ��������Ƿ���ͼ��߽���
bool inBorder(const cv::Point2f& pt)
{
    const int BORDER_SIZE = 1;
    //cvRound()�����ظ�������ӽ�������ֵ�����������룻
    int img_x = cvRound(pt.x);
    int img_y = cvRound(pt.y);
    return BORDER_SIZE <= img_x && img_x < COL - BORDER_SIZE && BORDER_SIZE <= img_y && img_y < ROW - BORDER_SIZE;
}

//ȥ���޷����ٵ�������
void reduceVector(vector<cv::Point2f>& v, vector<uchar> status)
{
    int j = 0;
    for (int i = 0; i < int(v.size()); i++)
        if (status[i])
            v[j++] = v[i];
    v.resize(j);
}

//ȥ���޷�׷�ٵ���������
void reduceVector(vector<int>& v, vector<uchar> status)
{
    int j = 0;
    for (int i = 0; i < int(v.size()); i++)
        if (status[i])
            v[j++] = v[i];
    v.resize(j);
}


FeatureTracker::FeatureTracker()
{
}

/**
 * @brief   �Ը��ٵ��������ȥ���ܼ���
 * @Description �Ը��ٵ��������㣬���ձ�׷�ٵ��Ĵ�����������ѡ��
 *              ʹ��mask�������ƷǼ������ƣ��뾶Ϊ30��ȥ���ܼ��㣬ʹ������ֲ�����
 * @return      void
*/
void FeatureTracker::setMask()
{
    if (FISHEYE)
        mask = fisheye_mask.clone();
    else
        mask = cv::Mat(ROW, COL, CV_8UC1, cv::Scalar(255));


    // prefer to keep features that are tracked for long time
    // ����(cnt��pts��id)����
    vector<pair<int, pair<cv::Point2f, int>>> cnt_pts_id;

    for (unsigned int i = 0; i < forw_pts.size(); i++)
        cnt_pts_id.push_back(make_pair(track_cnt[i], make_pair(forw_pts[i], ids[i])));

    //�Թ������ٵ���������forw_pts�����ձ����ٵ��Ĵ���cnt�Ӵ�С����lambda����ʽ��
    sort(cnt_pts_id.begin(), cnt_pts_id.end(), [](const pair<int, pair<cv::Point2f, int>>& a, const pair<int, pair<cv::Point2f, int>>& b)
        {
            return a.first > b.first;
        });

    //���cnt��pts��id�����´���
    forw_pts.clear();
    ids.clear();
    track_cnt.clear();

    for (auto& it : cnt_pts_id)
    {
        if (mask.at<uchar>(it.second.first) == 255)
        {
            //��ǰ������λ�ö�Ӧ��maskֵΪ255��������ǰ�����㣬����Ӧ��������λ��pts��id����׷�ٴ���cnt�ֱ����
            forw_pts.push_back(it.second.first);
            ids.push_back(it.second.second);
            track_cnt.push_back(it.first);
            //��mask�н���ǰ��������Χ�뾶ΪMIN_DIST����������Ϊ0�����治��ѡȡ�������ڵĵ㣨ʹ���ٵ㲻������һ�������ϣ�
            cv::circle(mask, it.second.first, MIN_DIST, 0, -1);
        }
    }
}

//�����¼�⵽��������n_pts
void FeatureTracker::addPoints()
{
    for (auto& p : n_pts)
    {
        forw_pts.push_back(p);
        ids.push_back(-1);//����ȡ��������id��ʼ��Ϊ-1
        track_cnt.push_back(1);//����ȡ�������㱻���ٵĴ�����ʼ��Ϊ1
    }
}

/**
 * @brief   ��ͼ��ʹ�ù������������������
 * @Description createCLAHE() ��ͼ���������Ӧֱ��ͼ���⻯
 *              calcOpticalFlowPyrLK() LK������������
 *              setMask() �Ը��ٵ������������mask
 *              rejectWithF() ͨ�����������޳�outliers
 *              goodFeaturesToTrack() ����������(shi-tomasi�ǵ�)��ȷ��ÿ֡�����㹻��������
 *              addPoints()�����µ�׷�ٵ�
 *              undistortedPoints() �Խǵ�ͼ������ȥ���������������ÿ���ǵ���ٶ�
 * @param[in]   _img ����ͼ��
 * @param[in]   _cur_time ��ǰʱ�䣨ͼ��ʱ�����
 * @return      void
*/
void FeatureTracker::readImage(const cv::Mat& _img, double _cur_time)
{
    cv::Mat img;
    cur_time = _cur_time;

    //���EQUALIZE=1����ʾ̫����̫��������ֱ��ͼ���⻯����
    if (EQUALIZE)
    {
        //����Ӧֱ��ͼ����
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(8, 8));
        //TicToc t_c;
        clahe->apply(_img, img);
        //ROS_DEBUG("CLAHE costs: %fms", t_c.toc());
    }
    else
        img = _img;

    if (forw_img.empty())
    {
        //�����ǰ֡��ͼ������forw_imgΪ�գ�˵����ǰ�ǵ�һ�ζ���ͼ������
        //�������ͼ�񸳸���ǰ֡forw_img��ͬʱ������prev_img��cur_img
        prev_img = cur_img = forw_img = img;
    }
    else
    {
        //����˵��֮ǰ���Ѿ���ͼ����룬ֻ��Ҫ���µ�ǰ֡forw_img������
        forw_img = img;
    }
    //��ʱforw_pts�����������һ֡ͼ���е������㣬���԰������
    forw_pts.clear();

    if (cur_pts.size() > 0)
    {
        //TicToc t_o;
        vector<uchar> status;
        vector<float> err;

        //����cv::calcOpticalFlowPyrLK()��ǰһ֡��������cur_pts����LK�������������٣��õ�forw_pts
        //status����˴�ǰһ֡cur_img��forw_img������ĸ���״̬���޷���׷�ٵ��ĵ���Ϊ0
        cv::calcOpticalFlowPyrLK(cur_img, forw_img, cur_pts, forw_pts, status, err, cv::Size(21, 21), 3);

        //��λ��ͼ��߽���ĵ���Ϊ0
        for (int i = 0; i < int(forw_pts.size()); i++)
            if (status[i] && !inBorder(forw_pts[i]))
                status[i] = 0;

        //����status,�Ѹ���ʧ�ܵĵ��޳�
        //����Ҫ�ӵ�ǰ֡����forw_pts���޳������һ�Ҫ��cur_un_pts��prev_pts��cur_pts���޳�
        //prev_pts��cur_pts�е���������һһ��Ӧ��
        //��¼������id��ids���ͼ�¼�����㱻���ٴ�����track_cntҲҪ�޳�
        reduceVector(prev_pts, status);
        reduceVector(cur_pts, status);
        reduceVector(forw_pts, status);
        reduceVector(ids, status);
        reduceVector(cur_un_pts, status);
        reduceVector(track_cnt, status);
        
    }

    //����׷�ٳɹ�,�����㱻�ɹ����ٵĴ����ͼ�1
    //��ֵ������׷�ٵĴ�������ֵԽ��˵����׷�ٵľ�Խ��
    for (auto& n : track_cnt)
        n++;

    //PUB_THIS_FRAME=1 ��Ҫ����������
    if (PUB_THIS_FRAME)
    {
        //ͨ�����������޳�outliers
        rejectWithF();
       
        setMask();//��֤���ڵ�������֮��Ҫ���30������,����mask

        //�����Ƿ���Ҫ��ȡ�µ�������
        int n_max_cnt = MAX_CNT - static_cast<int>(forw_pts.size());
        if (n_max_cnt > 0)
        {
            if (mask.empty())
                cout << "mask is empty " << endl;
            if (mask.type() != CV_8UC1)
                cout << "mask type wrong " << endl;
            if (mask.size() != forw_img.size())
                cout << "wrong size " << endl;
            /**
             *void cv::goodFeaturesToTrack(    ��mask�в�Ϊ0���������µ�������
             *   InputArray  image,              ����ͼ��
             *   OutputArray     corners,        ��ż�⵽�Ľǵ��vector
             *   int     maxCorners,             ���صĽǵ�����������ֵ
             *   double  qualityLevel,           �ǵ�����ˮƽ�������ֵ����ΧΪ0��1��������߽ǵ��ˮƽΪ1����С�ڸ���ֵ�Ľǵ㱻�ܾ�
             *   double  minDistance,            ���ؽǵ�֮��ŷʽ�������Сֵ
             *   InputArray  mask = noArray(),   ������ͼ�������ͬ��С�����ͱ���ΪCV_8UC1,��������ͼ���и���Ȥ������ֻ�ڸ���Ȥ�����м��ǵ�
             *   int     blockSize = 3,          ����Э�������ʱ�Ĵ��ڴ�С
             *   bool    useHarrisDetector = false,  ָʾ�Ƿ�ʹ��Harris�ǵ��⣬�粻ָ����ʹ��shi-tomasi�㷨
             *   double  k = 0.04                Harris�ǵ�����Ҫ��kֵ
             *)
             */
            cv::goodFeaturesToTrack(forw_img, n_pts, MAX_CNT - forw_pts.size(), 0.01, MIN_DIST, mask);
        }
        else
            n_pts.clear();

        //�����¼�⵽��������n_pts���ӵ�forw_pts�У�id��ʼ��-1,track_cnt��ʼ��Ϊ1.
        addPoints();
       
    }
    //����һ֡ͼ����ʱ����ǰ֡���ݾͳ�Ϊ����һ֡����������
    prev_img = cur_img;
    prev_pts = cur_pts;
    prev_un_pts = cur_un_pts;

    //�ѵ�ǰ֡������forw_img��forw_pts������һ֡cur_img��cur_pts
    cur_img = forw_img;
    cur_pts = forw_pts;

    //���ݲ�ͬ�����ģ��ȥ���������ת������һ������ϵ�ϣ������ٶ�
    undistortedPoints();
    prev_time = cur_time;
}

/**
 * @brief   ͨ��F����ȥ��outliers
 * @Description ��ͼ������ת��Ϊ��һ������
 *              cv::findFundamentalMat()����F����
 *              reduceVector()ȥ��outliers
 * @return      void
*/
void FeatureTracker::rejectWithF()
{
    if (forw_pts.size() >= 8)
    {
        vector<cv::Point2f> un_cur_pts(cur_pts.size()), un_forw_pts(forw_pts.size());
        for (unsigned int i = 0; i < cur_pts.size(); i++)
        {
            Eigen::Vector3d tmp_p;
            //���ݲ�ͬ�����ģ�ͽ���ά����ת������ά����
            m_camera->liftProjective(Eigen::Vector2d(cur_pts[i].x, cur_pts[i].y), tmp_p);
            //ת��Ϊ��һ����������
            tmp_p.x() = FOCAL_LENGTH * tmp_p.x() / tmp_p.z() + COL / 2.0;
            tmp_p.y() = FOCAL_LENGTH * tmp_p.y() / tmp_p.z() + ROW / 2.0;
            un_cur_pts[i] = cv::Point2f(tmp_p.x(), tmp_p.y());

            m_camera->liftProjective(Eigen::Vector2d(forw_pts[i].x, forw_pts[i].y), tmp_p);
            tmp_p.x() = FOCAL_LENGTH * tmp_p.x() / tmp_p.z() + COL / 2.0;
            tmp_p.y() = FOCAL_LENGTH * tmp_p.y() / tmp_p.z() + ROW / 2.0;
            un_forw_pts[i] = cv::Point2f(tmp_p.x(), tmp_p.y());
        }

        vector<uchar> status;
        //����cv::findFundamentalMat��un_cur_pts��un_forw_pts����F����
        cv::findFundamentalMat(un_cur_pts, un_forw_pts, cv::FM_RANSAC, F_THRESHOLD, 0.99, status);
        int size_a = cur_pts.size();
        reduceVector(prev_pts, status);
        reduceVector(cur_pts, status);
        reduceVector(forw_pts, status);
        reduceVector(cur_un_pts, status);
        reduceVector(ids, status);
        reduceVector(track_cnt, status);
    }
}

//����������id
bool FeatureTracker::updateID(unsigned int i)
{
    if (i < ids.size())
    {
        if (ids[i] == -1)
            ids[i] = n_id++;
        return true;
    }
    else
        return false;
}

//��ȡ����ڲ�
void FeatureTracker::readIntrinsicParameter(const string& calib_file)
{
    cout << "reading paramerter of camera " << calib_file << endl;
    m_camera = camodocal::CameraFactory::instance()->generateCameraFromYamlFile(calib_file);
}

//��ʾȥ����������������  nameΪͼ��֡����
void FeatureTracker::showUndistortion(const string& name)
{
    cv::Mat undistortedImg(ROW + 600, COL + 600, CV_8UC1, cv::Scalar(0));
    vector<Eigen::Vector2d> distortedp, undistortedp;
    for (int i = 0; i < COL; i++)
        for (int j = 0; j < ROW; j++)
        {
            Eigen::Vector2d a(i, j);
            Eigen::Vector3d b;
            m_camera->liftProjective(a, b);
            distortedp.push_back(a);
            undistortedp.push_back(Eigen::Vector2d(b.x() / b.z(), b.y() / b.z()));
        }
    for (int i = 0; i < int(undistortedp.size()); i++)
    {
        cv::Mat pp(3, 1, CV_32FC1);
        pp.at<float>(0, 0) = undistortedp[i].x() * FOCAL_LENGTH + COL / 2;
        pp.at<float>(1, 0) = undistortedp[i].y() * FOCAL_LENGTH + ROW / 2;
        pp.at<float>(2, 0) = 1.0;
        if (pp.at<float>(1, 0) + 300 >= 0 && pp.at<float>(1, 0) + 300 < ROW + 600 && pp.at<float>(0, 0) + 300 >= 0 && pp.at<float>(0, 0) + 300 < COL + 600)
        {
            undistortedImg.at<uchar>(pp.at<float>(1, 0) + 300, pp.at<float>(0, 0) + 300) = cur_img.at<uchar>(distortedp[i].y(), distortedp[i].x());
        }
        else
        {
        }
    }
    cv::imshow(name, undistortedImg);
    cv::waitKey(0);
}

//�Խǵ�ͼ���������ȥ���������ת������һ������ϵ�ϣ�������ÿ���ǵ���ٶ�
void FeatureTracker::undistortedPoints()
{
    cur_un_pts.clear();
    cur_un_pts_map.clear();
    for (unsigned int i = 0; i < cur_pts.size(); i++)
    {
        Eigen::Vector2d a(cur_pts[i].x, cur_pts[i].y);
        Eigen::Vector3d b;

        //���ݲ�ͬ�����ģ�ͽ���ά����ת������ά����
        m_camera->liftProjective(a, b);
        //�����쵽��ȹ�һ��ƽ����
        cur_un_pts.push_back(cv::Point2f(b.x() / b.z(), b.y() / b.z()));
        cur_un_pts_map.insert(make_pair(ids[i], cv::Point2f(b.x() / b.z(), b.y() / b.z())));
    }
    // ����ÿ����������ٶȵ�pts_velocity
    if (!prev_un_pts_map.empty())
    {
        double dt = cur_time - prev_time;
        pts_velocity.clear();
        for (unsigned int i = 0; i < cur_un_pts.size(); i++)
        {
            if (ids[i] != -1)
            {
                std::map<int, cv::Point2f>::iterator it;
                it = prev_un_pts_map.find(ids[i]);
                if (it != prev_un_pts_map.end())
                {
                    double v_x = (cur_un_pts[i].x - it->second.x) / dt;
                    double v_y = (cur_un_pts[i].y - it->second.y) / dt;
                    pts_velocity.push_back(cv::Point2f(v_x, v_y));
                }
                else
                    pts_velocity.push_back(cv::Point2f(0, 0));
            }
            else
            {
                pts_velocity.push_back(cv::Point2f(0, 0));
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < cur_pts.size(); i++)
        {
            pts_velocity.push_back(cv::Point2f(0, 0));
        }
    }
    prev_un_pts_map = cur_un_pts_map;
}