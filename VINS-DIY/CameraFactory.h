#pragma once

#include <memory>
#include <string>
#include "Camera.h"

namespace camodocal
{

    class CameraFactory
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            CameraFactory();

        static std::shared_ptr<CameraFactory> instance(void);

        CameraPtr generateCamera(Camera::ModelType modelType,
            const std::string& cameraName,
            cv::Size imageSize) const;

        CameraPtr generateCameraFromYamlFile(const std::string& filename);

    private:
        static std::shared_ptr<CameraFactory> m_instance;
    };

}
