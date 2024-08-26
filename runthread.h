#ifndef __RUN_THREAD_H__
#define __RUN_THREAD_H__

#include "graph.h"
#include <memory>
#include <thread>
#include <atomic>
#include <QImage>
#include <QColor>
#include <vector>

class run_thread {
public:

    ~run_thread();

    run_thread(const QImage& image, const std::vector<std::pair<QColor, QColor>>& cmap, const double learning_rate);

    void stop() {
        run_ = false;
    }

    QImage result() const {
        return result_;
    }

    std::string result_string() const {
        return result_string_;
    }

    bool is_running() const {
        return run_;
    }

    bool is_done() const {
        return done_;
    }

    double get_last_error() const {
        return error_;
    }

    double get_best_error() const {
        return best_error_;
    }

private:

    void thread_function();

    QImage image_;
    QImage result_;
    std::string result_string_;
    std::vector<std::pair<QColor, QColor>> cmap_;
    double learning_rate_;
    std::atomic<double> error_;
    std::atomic<double> best_error_;
    std::atomic<bool> run_;
    std::atomic<bool> abort_;
    std::atomic<bool> done_;
    std::shared_ptr<std::thread> thread_;
};

#endif /* __RUN_THREAD_H__ */
