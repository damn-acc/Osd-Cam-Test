#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>

class Compass {
    private:
        std::atomic<float> angle{270.0f}; // Start position (pointing up)

        int thickness;
        int radius;
        bool is_ccw;
        float delta_angle; // degrees per second
        cv::Scalar color; 
        cv::Point2f center;
        mutable std::mutex mtx;
        cv::Point2f arrow_tip;        
        std::atomic<bool> running{false};
        std::thread thrd;

        void threadLoop() {
            const int sleep_ms = 16; // 62.5 updates per second
            const float sleep_s = sleep_ms / 1000.0f;

            float step = delta_angle * sleep_s;

            while (running.load()) {
                float curr_angle = angle.load();
                curr_angle += step;
                if (curr_angle >= 360.0f) {
                    curr_angle -= 360.0f;
                }
                if (curr_angle < 0.0f) {
                    curr_angle += 360.0f;
                }
                angle.store(curr_angle);

                float rad = curr_angle * (float)CV_PI / 180.0f;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    arrow_tip.x = center.x + radius * cos(rad);
                    arrow_tip.y = center.y + radius * sin(rad);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
        }

    public:
        Compass(cv::Point c, int r, cv::Scalar col, float speed, int thickness, bool is_ccw) 
        : center(c), radius(r), color(col), delta_angle(speed), thickness(thickness), is_ccw(is_ccw) {
            if (is_ccw) {
                delta_angle *= (float)-1;
            }
            arrow_tip = cv::Point2f(c.x, c.y - r);
        }

        void start() {
            if (!running.load()) {
                running.store(true);
                thrd = std::thread(&Compass::threadLoop, this);
            }
        }

        void stop() {
            running.store(false);
            if (thrd.joinable()) {
                thrd.join();
            }
        }

        ~Compass() {
            stop();
        }

        void draw(cv::Mat& frame) const {
            cv::Point2f local_tip;
            {
                std::lock_guard<std::mutex> lock(mtx);
                local_tip = arrow_tip;
            }
            cv::circle(frame, center, radius, color, thickness);
            cv::arrowedLine(frame, center, local_tip, color, thickness);
        }
};