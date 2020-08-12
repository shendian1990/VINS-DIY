#include "CameraFactory.h"
#include <iostream>
#include "PinholeCamera.h"

namespace camodocal
{

    std::shared_ptr<CameraFactory> CameraFactory::m_instance;

    CameraFactory::CameraFactory()
    {

    }

    std::shared_ptr<CameraFactory>
        CameraFactory::instance(void)
    {
        if (m_instance.get() == 0)
        {
            m_instance.reset(new CameraFactory);
        }

        return m_instance;
    }

    CameraPtr
        CameraFactory::generateCamera(Camera::ModelType modelType,
            const std::string& cameraName,
            cv::Size imageSize) const
    {
        switch (modelType)
        {
        case Camera::ModelType::PINHOLE:
        {
            PinholeCameraPtr camera(new PinholeCamera);

            PinholeCamera::Parameters params = camera->getParameters();
            params.cameraName() = cameraName;
            params.imageWidth() = imageSize.width;
            params.imageHeight() = imageSize.height;
            camera->setParameters(params);
            return camera;
        }
        default:
        {
            PinholeCameraPtr camera(new PinholeCamera);

            PinholeCamera::Parameters params = camera->getParameters();
            params.cameraName() = cameraName;
            params.imageWidth() = imageSize.width;
            params.imageHeight() = imageSize.height;
            camera->setParameters(params);
            return camera;
        }
        }
    }

    CameraPtr
        CameraFactory::generateCameraFromYamlFile(const std::string& filename)
    {
        cv::FileStorage fs(filename, cv::FileStorage::READ);

        if (!fs.isOpened())
        {
            return CameraPtr();
        }

        Camera::ModelType modelType = Camera::ModelType::MEI;
        if (!fs["model_type"].isNone())
        {
            std::string sModelType;
            fs["model_type"] >> sModelType;

            if (sModelType.find("PINHOLE") != std::string::npos)
            {
                modelType = Camera::ModelType::PINHOLE;
            }
            else
            {
                std::cerr << "# ERROR: Unknown camera model: " << sModelType << std::endl;
                return CameraPtr();
            }
        }

        switch (modelType)
        {
        case Camera::ModelType::PINHOLE:
        {
            PinholeCameraPtr camera(new PinholeCamera);

            PinholeCamera::Parameters params = camera->getParameters();
            params.readFromYamlFile(filename);
            camera->setParameters(params);
            return camera;
        }
        default:
        {
            PinholeCameraPtr camera(new PinholeCamera);

            PinholeCamera::Parameters params = camera->getParameters();
            params.readFromYamlFile(filename);
            camera->setParameters(params);
            return camera;
        }
        }
    }

}

