#include <iostream>
#include <memory>
#include <set>
#include <deque>
#include <map>
#include <cmath>
#include <vector>
#include <QImage>
#include <QColor>

class graph_node {
public:
  virtual ~graph_node() = default;
};

class graph_variable : public graph_node {
public:
};

class graph_binary : public graph_node {
public:
  graph_binary(const std::shared_ptr<graph_node>& lhs, const std::shared_ptr<graph_node> rhs) : lhs_(lhs), rhs_(rhs) {
  }
  std::shared_ptr<graph_node> left() const {
    return lhs_;
  }
  std::shared_ptr<graph_node> right() const {
    return rhs_;
  }
  virtual double evaluate(const double lhs_value, const double rhs_value) = 0;
private:
  std::shared_ptr<graph_node> lhs_;
  std::shared_ptr<graph_node> rhs_;
};

class graph_add : public graph_binary {
public:
  graph_add(const std::shared_ptr<graph_node>& lhs, const std::shared_ptr<graph_node> rhs) : graph_binary(lhs, rhs) {
  }
  double evaluate(const double lhs_value, const double rhs_value) override {
    return lhs_value + rhs_value;
  }
};

class graph_sub : public graph_binary {
public:
  graph_sub(const std::shared_ptr<graph_node>& lhs, const std::shared_ptr<graph_node> rhs) : graph_binary(lhs, rhs) {
  }
  double evaluate(const double lhs_value, const double rhs_value) override {
    return lhs_value - rhs_value;
  }
};

class graph_mul : public graph_binary {
public:
  graph_mul(const std::shared_ptr<graph_node>& lhs, const std::shared_ptr<graph_node> rhs) : graph_binary(lhs, rhs) {
  }
  double evaluate(const double lhs_value, const double rhs_value) override {
    return lhs_value * rhs_value;
  }
};

class graph_unary : public graph_node {
public:
  graph_unary(const std::shared_ptr<graph_node>& input) : input_(input) {
  }
  std::shared_ptr<graph_node> input() const {
    return input_;
  }
  virtual double evaluate(const double value) = 0;
  virtual double evaluate_delta(const double value) = 0;
private:
  std::shared_ptr<graph_node> input_;
};

class graph_sqrt : public graph_unary {
public:
  graph_sqrt(const std::shared_ptr<graph_node>& input) : graph_unary(input) {
  }
  double evaluate(const double value) override {
    return sqrt(value);
  }
  double evaluate_delta(const double value) override {
    /* d/dx x^(1/2) = (1/2) x^(-1/2) */
    return 1 / (2 * sqrt(value));
  }
};

class graph_tanh : public graph_unary {
public:
  graph_tanh(const std::shared_ptr<graph_node>& input) : graph_unary(input) {
  }
  double evaluate(const double value) override {
    return tanh(value);
  }
  double evaluate_delta(const double value) override {
    /* d/dx tanh(x) = 1 - tanh^2(x) */
    return 1 - tanh(value) * tanh(value);
  }
};

class graph_builder {
public:
  graph_builder() {
  }
  graph_builder(const std::shared_ptr<graph_node>& node) : root_(node) {
  }
  bool empty() const {
    return !root_;
  }
  std::shared_ptr<graph_node> root() const {
    return root_;
  }
  graph_builder operator +(const graph_builder& rhs) const {
    return graph_builder(std::make_shared<graph_add>(root(), rhs.root()));
  }
  graph_builder operator -(const graph_builder& rhs) const {
    return graph_builder(std::make_shared<graph_sub>(root(), rhs.root()));
  }
  graph_builder operator *(const graph_builder& rhs) const {
    return graph_builder(std::make_shared<graph_mul>(root(), rhs.root()));
  }
  static graph_builder sqrt(const graph_builder& input) {
    return graph_builder(std::make_shared<graph_sqrt>(input.root()));
  }
  static graph_builder tanh(const graph_builder& input) {
    return graph_builder(std::make_shared<graph_tanh>(input.root()));
  }
private:
  std::shared_ptr<graph_node> root_;
};

class graph_evaluator {
public:
  graph_builder constant(const double value) {
    std::shared_ptr<graph_variable> variable = std::make_shared<graph_variable>();
    values_.emplace(variable, value);
    return graph_builder(variable);
  }
  graph_builder parameter() {
    std::shared_ptr<graph_variable> variable = std::make_shared<graph_variable>();
    values_.emplace(variable, 0.0);
    return graph_builder(variable);
  }
  double get_parameter(const graph_builder& graph) const {
    auto map_item = values_.find(graph.root());
    if (map_item == std::end(values_)) {
      throw std::runtime_error("Parameter is not registered");
    }
    return map_item->second;
  }
  void set_parameter(const graph_builder& parameter, const double value) {
    auto map_item = values_.find(parameter.root());
    if (map_item == std::end(values_)) {
      throw std::runtime_error("Parameter is not registered");
    }
    map_item->second = value;
  }
  double evaluate(const graph_builder& graph) {
    std::map<std::shared_ptr<graph_node>, double> new_values;
    for (auto value : values_) {
      if (std::dynamic_pointer_cast<graph_variable>(value.first)) {
        new_values.emplace(value.first, value.second);
      }
    }
    values_ = new_values;
    std::deque<std::shared_ptr<graph_node>> stack;
    stack.push_back(graph.root());
    while (!stack.empty()) {
      std::shared_ptr<graph_node> top = stack.back();
      if (std::dynamic_pointer_cast<graph_variable>(top)) {
        stack.pop_back();
      } else if (std::dynamic_pointer_cast<graph_unary>(top)) {
        std::shared_ptr<graph_unary> unary = std::dynamic_pointer_cast<graph_unary>(top);
        auto input_item = values_.find(unary->input());
        if (input_item == std::end(values_)) {
          stack.push_back(unary->input());
        } else {
          stack.pop_back();
          values_[unary] = unary->evaluate(input_item->second);
        }
      } else if (std::dynamic_pointer_cast<graph_binary>(top)) {
        std::shared_ptr<graph_binary> binary = std::dynamic_pointer_cast<graph_binary>(top);
        auto lhs_item = values_.find(binary->left());
        auto rhs_item = values_.find(binary->right());
        if ((lhs_item == std::end(values_)) && (rhs_item == std::end(values_))) {
          stack.push_back(binary->left());
          stack.push_back(binary->right());
        } else if (lhs_item == std::end(values_)) {
          stack.push_back(binary->left());
        } else if (rhs_item == std::end(values_)) {
          stack.push_back(binary->right());
        } else {
          stack.pop_back();
          values_[binary] = binary->evaluate(lhs_item->second, rhs_item->second);
        }
      }
    }
    return values_[graph.root()];
  }
  double evaluate_delta(const graph_builder& graph, const graph_builder& parameter) const {
    std::map<std::shared_ptr<graph_node>, double> deltas;
    std::deque<std::shared_ptr<graph_node>> stack;
    stack.push_back(graph.root());
    while (!stack.empty()) {
      std::shared_ptr<graph_node> top = stack.back();
      if (std::dynamic_pointer_cast<graph_variable>(top)) {
        stack.pop_back();
        if (top == parameter.root()) {
          deltas[top] = 1; /* d/dx x = 1 */
        } else {
          deltas[top] = 0; /* d/dx y = 0 */
        }
      } else if (std::dynamic_pointer_cast<graph_unary>(top)) {
        std::shared_ptr<graph_unary> unary = std::dynamic_pointer_cast<graph_unary>(top);
        auto input_item = deltas.find(unary->input());
        if (input_item == std::end(deltas)) {
          stack.push_back(unary->input());
        } else {
          stack.pop_back();
          /* d/dx f(g(x)) = f'(g(x))g'(x) */
          deltas[unary] += unary->evaluate_delta(get_value(unary->input())) * deltas[unary->input()];
        }
      } else if (std::dynamic_pointer_cast<graph_binary>(top)) {
        std::shared_ptr<graph_binary> binary = std::dynamic_pointer_cast<graph_binary>(top);
        auto lhs_item = deltas.find(binary->left());
        auto rhs_item = deltas.find(binary->right());
        if ((lhs_item == std::end(deltas)) && (rhs_item == std::end(deltas))) {
          stack.push_back(binary->left());
          stack.push_back(binary->right());
        } else if (lhs_item == std::end(deltas)) {
          stack.push_back(binary->left());
        } else if (rhs_item == std::end(deltas)) {
          stack.push_back(binary->right());
        } else {
          stack.pop_back();
          if (std::dynamic_pointer_cast<graph_add>(binary)) {
            /* d/dx (f(x) + g(x)) = d/dx f(x) + d/dx g(x) */
            deltas[top] += deltas[binary->left()] + deltas[binary->right()];
          } else if (std::dynamic_pointer_cast<graph_sub>(binary)) {
            /* d/dx (f(x) - g(x)) = d/dx f(x) - d/dx g(x) */
            deltas[top] += deltas[binary->left()] - deltas[binary->right()];
          } else if (std::dynamic_pointer_cast<graph_mul>(binary)) {
            /* d/dx (f(x) * g(x)) = f(x)g'(x) + f'(x)g(x) */
            deltas[top] += get_value(binary->left()) * deltas[binary->right()];
            deltas[top] += deltas[binary->left()] * get_value(binary->right());
          } else {
            throw std::runtime_error("Node type not implemented");
          }
        }
      }
    }
    return deltas[graph.root()];
  }
private:
  double get_value(const std::shared_ptr<graph_node>& node) const {
      auto map_item = values_.find(node);
      if (map_item == std::end(values_)) {
          return 0.0;
      } else {
          return map_item->second;
      }
  }
  std::map<std::shared_ptr<graph_node>, double> values_;
};

class bp_layer {
public:
     bp_layer(graph_evaluator& bp, const std::vector<graph_builder>& inputs, const size_t num_outputs, const bool final_layer) {
        const size_t num_weights = inputs.size() + 1; /* plus bias */
        std::vector<graph_builder> parameters(num_outputs * num_weights);
        for (size_t i = 0; i < parameters.size(); ++i) {
          parameters[i] = bp.parameter();
        }

        std::vector<graph_builder> outputs(num_outputs);
        for (size_t i = 0; i < outputs.size(); ++i) {
          graph_builder out = parameters[i * num_weights + (num_weights - 1)];
          for (size_t j = 0; j < inputs.size(); ++j) {
            out = out + inputs[j] * parameters[i * num_weights + j];
          }
          if (final_layer) {
            outputs[i] = out;
          } else {
            outputs[i] = graph_builder::tanh(out);
          }
        }

        parameters_ = parameters;
        outputs_ = outputs;
    }
    std::vector<graph_builder> parameters_;
    std::vector<graph_builder> outputs_;
};
