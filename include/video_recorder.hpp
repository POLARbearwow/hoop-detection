#pragma once
#include <opencv2/opencv.hpp>
#include "hik_camera.hpp"
#include <string>

class VideoRecorder {
public:
    explicit VideoRecorder(const std::string& save_dir = "saved_videos");
    ~VideoRecorder() = default;

    // 运行录制流程
    bool run();

private:
    // 创建保存目录
    bool createSaveDirectory();

    // 生成文件名
    std::string generateFilename() const;

private:
    std::string save_dir_;
    HikCamera camera_;
}; 