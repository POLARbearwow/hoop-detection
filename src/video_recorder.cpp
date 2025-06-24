#include "video_recorder.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

VideoRecorder::VideoRecorder(const std::string& save_dir)
    : save_dir_(save_dir) {
    std::cout << "[VideoRecorder] 初始化，保存目录: " << save_dir_ << std::endl;
}

bool VideoRecorder::createSaveDirectory() {
    try {
        if (!std::filesystem::exists(save_dir_)) {
            if (!std::filesystem::create_directories(save_dir_)) {
                std::cerr << "[VideoRecorder] 创建目录失败: " << save_dir_ << std::endl;
                return false;
            }
            std::cout << "[VideoRecorder] 创建目录: " << save_dir_ << std::endl;
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[VideoRecorder] 文件系统错误: " << e.what() << std::endl;
        return false;
    }
}

std::string VideoRecorder::generateFilename() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << save_dir_ << "/";
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    ss << "_" << std::setw(3) << std::setfill('0') << ms.count();
    ss << ".avi";
    return ss.str();
}

bool VideoRecorder::run() {
    if (!createSaveDirectory()) return false;

    if (!camera_.openCamera()) {
        std::cerr << "[VideoRecorder] 相机初始化失败" << std::endl;
        return false;
    }

    cv::Mat frame;
    if (!camera_.getFrame(frame) || frame.empty()) {
        std::cerr << "[VideoRecorder] 无法从相机获取帧" << std::endl;
        return false;
    }

    int width = frame.cols;
    int height = frame.rows;
    double fps = 30.0; // 估计帧率
    int fourcc = cv::VideoWriter::fourcc('M','J','P','G');

    bool recording = false;
    cv::VideoWriter writer;

    cv::namedWindow("Record", cv::WINDOW_NORMAL);
    cv::resizeWindow("Record", 960, 540);
    std::cout << "\n=== 视频录制模式 ===" << std::endl;
    std::cout << "按's'开始/停止录制" << std::endl;
    std::cout << "按'q'退出程序" << std::endl;

    char key = 0;
    while (key != 'q') {
        if (!camera_.getFrame(frame)) {
            std::cerr << "[VideoRecorder] 获取图像失败" << std::endl;
            continue;
        }

        if (recording && writer.isOpened()) {
            writer.write(frame);
        }

        // 显示状态文本
        cv::Mat display = frame.clone();
        std::string text = recording ? "REC" : "PAUSE";
        cv::putText(display, text, cv::Point(30,60), cv::FONT_HERSHEY_SIMPLEX, 2,
                    recording ? cv::Scalar(0,0,255) : cv::Scalar(0,255,0), 3);
        cv::imshow("Record", display);

        key = cv::waitKey(1);
        if (key == 's' || key == 'S') {
            if (!recording) {
                std::string filename = generateFilename();
                writer.open(filename, fourcc, fps, cv::Size(width, height));
                if (!writer.isOpened()) {
                    std::cerr << "[VideoRecorder] 无法打开视频文件: " << filename << std::endl;
                } else {
                    std::cout << "[VideoRecorder] 开始录制: " << filename << std::endl;
                    recording = true;
                }
            } else {
                writer.release();
                recording = false;
                std::cout << "[VideoRecorder] 停止录制" << std::endl;
            }
        }
    }

    // 确保释放
    if (writer.isOpened()) {
        writer.release();
        std::cout << "[VideoRecorder] 已保存视频" << std::endl;
    }
    cv::destroyWindow("Record");
    return true;
} 