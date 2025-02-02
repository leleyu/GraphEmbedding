//
// Created by leleyu on 2019-04-03.
//

#include <angel/graph/model/graphsage.h>

namespace angel {
namespace graph {

SupervisedGraphSage::SupervisedGraphSage(int input_dim,
  int num_class,
  const std::vector<int32_t>& output_dims) {
  size_t n_layers = output_dims.size();
  for (size_t i = 0; i < n_layers; i++) {
    layers.emplace_back(register_module("layer" + std::to_string(i),
      Mean(input_dim, output_dims[i])));
    input_dim = output_dims[i];
  }

  weight_ = register_parameter("weight", torch::randn({input_dim, num_class}));
  torch::nn::init::xavier_uniform_(weight_);
}

torch::Tensor
SupervisedGraphSage::Forward(const torch::Tensor &nodes,
  const SubGraph &sub_graph,
  const torch::Tensor &input_embeddings) {
  auto output = ComputeOutput(nodes, layers.size() - 1,
    sub_graph, input_embeddings);
  // normalization
  auto norm = output.norm(2, 1)
    .view({-1, 1})
    .clamp_min(10e-15);
  output = output.div(norm);

  return relu(output.matmul(weight_));
}

torch::Tensor
SupervisedGraphSage::ComputeOutput(const torch::Tensor &nodes,
  int layer,
  const SubGraph &sub_graph,
  const torch::Tensor &input_embeddings) {

  if (layer > 0) {
    // compute the output of its neighbors and self
    auto first = sub_graph.GetFirstOrder(nodes);
    auto output = ComputeOutput(first, layer - 1, sub_graph, input_embeddings);
    return layers[layer]->Forward(nodes, sub_graph, first, output);
  } else {
    // use the input embeddings
    return layers[layer]->Forward(nodes, sub_graph, input_embeddings);
  }
}

std::map<std::string, torch::Tensor>
SupervisedGraphSage::Backward(const torch::Tensor &nodes,
  const angel::graph::SubGraph &sub_graph,
  const torch::Tensor &input_embeddings,
  const torch::Tensor &targets) {
  auto y_pred = Forward(nodes, sub_graph, input_embeddings);
  auto loss = torch::nll_loss(log_softmax(y_pred, 1), targets);
  loss.backward();
  // return weights grads

  std::map<std::string, torch::Tensor> grads;

  for (auto& item: named_parameters()) {
    auto& p = item.value();
    if (!p.grad().defined())
      continue;
    auto& key = item.key();
    grads.insert(std::make_pair(key, p.grad()));
  }

  return grads;
}

int32_t SupervisedGraphSage::GetDim() {
  return layers[0]->options.in();
}

std::vector<std::string> SupervisedGraphSage::keys() {
  std::vector<std::string> keys;
  for (auto& item: named_parameters()) {
    auto& key = item.key();
    keys.push_back(key);
  }

  return keys;
}


double SupervisedGraphSage::Fit(const torch::Tensor &nodes, const angel::graph::SubGraph &sub_graph,
                                const torch::Tensor &input_embeddings, const torch::Tensor &targets) {

}
} // namespace graph
} // namespace angel
