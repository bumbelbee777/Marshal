#pragma once
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

namespace Marshal::IO {

struct OutBatch {
    int index = 0;
    std::string payload;
};

class AsyncWriter {
public:
    explicit AsyncWriter(std::string path) : path_(std::move(path)) {
        worker_ = std::thread([this] { Run(); });
    }
    ~AsyncWriter() {
        {
            std::lock_guard<std::mutex> lk(mu_);
            done_ = true;
        }
        cv_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

    void Enqueue(OutBatch&& b) {
        std::lock_guard<std::mutex> lk(mu_);
        q_.push(std::move(b));
        cv_.notify_one();
    }

private:
    void Run() {
        std::ofstream out(path_, std::ios::app);
        for (;;) {
            OutBatch b;
            {
                std::unique_lock<std::mutex> lk(mu_);
                cv_.wait(lk, [this] { return done_ || !q_.empty(); });
                if (done_ && q_.empty()) break;
                b = std::move(q_.front());
                q_.pop();
            }
            if (!b.payload.empty()) out << b.payload;
            out.flush();
        }
    }

    std::string path_;
    std::mutex mu_;
    std::condition_variable cv_;
    std::queue<OutBatch> q_;
    bool done_ = false;
    std::thread worker_;
};

}  // namespace Marshal::IO
