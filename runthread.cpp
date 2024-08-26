#include "runthread.h"
#include <sstream>

run_thread::~run_thread() {
    run_ = false;
    abort_ = true;
    thread_->join();
}

run_thread::run_thread(const QImage& image, const std::vector<std::pair<QColor, QColor>>& cmap, const double learning_rate) : image_(image), cmap_(cmap), learning_rate_(learning_rate) {
    done_ = false;
    run_ = true;
    abort_ = false;
    thread_ = std::make_shared<std::thread>(std::bind(&run_thread::thread_function, this));
}

void run_thread::thread_function() {

    graph_evaluator bp;

    std::vector<graph_builder> input(3);
    for (size_t i = 0; i < input.size(); ++i) {
      input[i] = bp.parameter();
    }

    bp_layer layer1(bp, input, 4, false);
    bp_layer output_layer(bp, layer1.outputs_, 3, true);

    std::vector<graph_builder> all_params;
    all_params.insert(std::end(all_params), std::begin(layer1.parameters_), std::end(layer1.parameters_));
    all_params.insert(std::end(all_params), std::begin(output_layer.parameters_), std::end(output_layer.parameters_));

    for (size_t i = 0; i < all_params.size(); ++i) {
      bp.set_parameter(all_params[i], (rand() / double(RAND_MAX) * 2 - 1));
    }

    graph_builder error;
    std::vector<graph_builder> cmatch(3);
    for (size_t i = 0; i < cmatch.size(); ++i) {
      cmatch[i] = bp.parameter();
      graph_builder e = output_layer.outputs_[i] - cmatch[i];
      e = e * e;
      if (error.empty()) {
          error = e;
      } else {
          error = error + e;
      }
    }

    const double lr = learning_rate_;

    graph_evaluator best_bp;
    best_error_ = std::numeric_limits<double>::max();

    size_t iteration = 0;
    while (run_ == true && abort_ == false) {
      iteration++;
      QColor col_in = cmap_[iteration % cmap_.size()].first;
      QColor col_out = cmap_[iteration % cmap_.size()].second;

      bp.set_parameter(input[0], col_in.redF());
      bp.set_parameter(input[1], col_in.greenF());
      bp.set_parameter(input[2], col_in.blueF());

      bp.set_parameter(cmatch[0], col_out.redF());
      bp.set_parameter(cmatch[1], col_out.greenF());
      bp.set_parameter(cmatch[2], col_out.blueF());

      double e = bp.evaluate(error);
      error_ = e;

      if (e < best_error_) {
          best_bp = bp;
          best_error_ = e;
      }

      std::vector<double> deltas(all_params.size());
      #pragma parallel for
      for (size_t j = 0; j < all_params.size(); ++j) {
          deltas[j] = bp.evaluate_delta(error, all_params[j]);
      }

      for (size_t j = 0; j < all_params.size(); ++j) {
          double c = bp.get_parameter(all_params[j]);
          bp.set_parameter(all_params[j], c - deltas[j] * e * lr);
      }
    }

    bp = best_bp;

    QImage new_image = image_;
    #pragma parallel for
    for (int y = 0; y < new_image.height(); ++y) {
        graph_evaluator omp_bp = bp;
        for (int x = 0; x < new_image.width(); ++x) {
          if (abort_ == true) {
            break;
          }

          QColor current = new_image.pixel(x, y);
          omp_bp.set_parameter(input[0], current.redF());
          omp_bp.set_parameter(input[1], current.greenF());
          omp_bp.set_parameter(input[2], current.blueF());

          omp_bp.evaluate(error);

          QColor new_colour;
          new_colour.setRedF(omp_bp.get_parameter(output_layer.outputs_[0]));
          new_colour.setGreenF(omp_bp.get_parameter(output_layer.outputs_[1]));
          new_colour.setBlueF(omp_bp.get_parameter(output_layer.outputs_[2]));

          new_image.setPixel(x, y, new_colour.rgb());
        }
    }

    std::stringstream ss;
    ss << "mat4x4 a = mat4x4(";
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            if (i != 0 || j != 0) {
                ss << ", ";
            }
            ss << bp.get_parameter(layer1.parameters_[j * 4 + i]);
        }
    }
    ss << ");" << std::endl;

    ss << "mat4x3 b = mat4x3(";
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (i != 0 || j != 0) {
                ss << ", ";
            }
            ss << bp.get_parameter(output_layer.parameters_[j * 5 + i]);
        }
    }
    ss << ");" << std::endl;

    ss << "vec3 c = vec3(";
    for (size_t i = 0; i < 3; ++i) {
        if (i != 0) {
            ss << ", ";
        }
        ss << bp.get_parameter(output_layer.parameters_[i * 5 + 4]);
    }
    ss << ");" << std::endl;

    ss << "col = b * tanh(a * vec4(col, 1.0)) + c;";

    result_string_ = ss.str();

    result_ = new_image;
    done_ = true;
}
