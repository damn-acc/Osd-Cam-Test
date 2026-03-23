#include <opencv2/opencv.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include "compass.cpp"

using json = nlohmann::json;

struct Config {
    int width, height;
    float compass_speed;
    cv::Scalar color;
    std::string osd_text;
    std::string camera;
    bool is_comp_ccw;

    void save(std::string path) {
        json data;
        data["resolution"] = {width, height};
        data["compass_speed"] = compass_speed;
        data["color"] = {color[0], color[1], color[2]};
        data["osd_text"] = osd_text;
        data["cam_path"] = camera;
        data["is_comp_ccw"] = is_comp_ccw;

        std::ofstream f(path);
        f << data.dump(4);
    }

    static Config load(std::string path) {
        std::ifstream f(path);
        if (!f.is_open()) {
            std::cerr << "Failed to open config file: " << path << std::endl;
            exit(-1);
        }

        json data = json::parse(f);
        Config c;
        c.width = data["resolution"][0];
        c.height = data["resolution"][1];
        c.color = cv::Scalar(data["color"][0], data["color"][1], data["color"][2]);
        c.osd_text = data["osd_text"];
        c.compass_speed = data["compass_speed"];
        c.camera = data.value("cam_path", " ");
        c.is_comp_ccw = data.value("is_comp_ccw", false);

        return c;
    }
};


int main(int argc, char** argv) {
    std::string const conf_path = "config.json";
    std::string const win_name = "VGU_TEST";
    int const max_cam_width = 1280;
    int const min_cam_width = 640;

    bool is_comp_ccw;
    std::string cam_path = "";

    {
        const char* keys = 
        "{h help | | Show help message}"
        "{c camera | | Camera path (default: /dev/video0)}"
        "{cr compass-reverse | | Reverse compass rotation direction}";

        cv::CommandLineParser parser(argc, argv, keys);
        if (parser.has("help")) {
            parser.printMessage();
            return 0;
        }

        cam_path = parser.get<std::string>("camera");

        is_comp_ccw = parser.has("compass-reverse");

        if (!parser.check()) {
            parser.printErrors();
            return -1;
        }

        if (cam_path.empty()) {
            std::cerr << "Camera path not specified" << std::endl;
            return -1;
        }
    }
    
    Config config = Config::load(conf_path);
    config.camera = cam_path;
    config.is_comp_ccw = is_comp_ccw;
    config.save("config.json");

    cv::VideoCapture cap(cam_path, cv::CAP_V4L2); 
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera" << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, config.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, config.height);

    cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);

    cv::Mat frame;

    // osd positioning ---------------------
    float font_scale = config.width / (float)max_cam_width;
    int thickness = std::max(1, (int)(config.width / min_cam_width));

    int pad_x = config.width * 0.02;
    int pad_y = config.height * 0.02;

    int comp_radius = config.height * 0.2;

    cv::Point text_pos(pad_x, config.height - pad_y);
    cv::Point compass_pos(config.width - comp_radius - pad_x, config.height - comp_radius - pad_y);
    // -------------------------------------

    Compass compass(compass_pos, comp_radius, config.color, config.compass_speed, thickness, is_comp_ccw);
    compass.start();

    while (true) {
        if (!cap.read(frame)) {
            std::cerr << "Failed to capture frame" << std::endl;
            break;
        }

        cv::putText(
            frame,
            config.osd_text, 
            text_pos, 
            cv::FONT_HERSHEY_SIMPLEX, 
            font_scale, 
            config.color, 
            thickness
        );
        compass.draw(frame);
        
        cv::imshow(win_name, frame);

        if (cv::waitKey(1) == 27) {
            break;
        } 
    }

    compass.stop();
    return 0;
}