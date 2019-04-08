//
// Created by leleyu on 19-1-10.
//
#include <iostream>
#include <torch/torch.h>
#include <graph/graphsage.h>
#include <graph/metric.h>

void TrainUnSupervisedGraphSage(graph::nn::UnSupervisedGraphsage *net,
                                const graph::Graph &graph,
                                size_t batch_size,
                                size_t num_epoch) {
  auto dataset = graph::EdgeDataset(graph.GetEdgeInfo());
  auto options = torch::data::DataLoaderOptions(batch_size);
  auto sampler = torch::data::samplers::RandomSampler(dataset.size().value());
  auto loader = torch::data::make_data_loader(dataset, options, sampler);
  
  torch::optim::SGD optim(net->parameters(), 0.01);

  for (size_t epoch = 1; epoch <= num_epoch; epoch++) {
    double loss_sum = 0.0;
    size_t batch_index = 0;
    for (auto batch : *loader) {
      auto output = net->Forward(batch);
      int64_t batch_size = batch.size();
      auto srcs = output.slice(0, 0, batch_size / 2);
      auto dsts = output.slice(0, batch_size / 2, batch_size);
      auto loss = net->PairwiseLoss(srcs, dsts);
      loss.backward();
      optim.step();
      loss_sum += loss.item().toDouble() * batch.size() / 2;

      std::cout << "epoch " << epoch << " "
                << "batch_index=" << batch_index++ << " "
                << "batch_size=" << batch_size << " "
                << "loss=" << loss.item().toFloat()
                << std::endl;
    }

    std::cout << "epoch " << epoch << " "
              << "loss=" << loss_sum / dataset.size().value()
              << std::endl;
  }
}

void RunUnSupervisedGraphSage(const graph::Graph &graph) {
  graph::sampler::UniformSampler sampler;

  int64_t input_dim = graph.GetInputEmbedding().GetDim();
  graph::nn::UnSupervisedGraphsage net(input_dim, graph,
                                       {20, 10}, {5, 5}, sampler);

  TrainUnSupervisedGraphSage(&net, graph, 1024, 5);
}

void RunCora() {
  const std::string edge_path = "../data/cora/cora.edge";
  const std::string feature_path = "../data/cora/cora.feature";

  int64_t n_node = 2708;
//  int64_t n_dim = 1433;
  int64_t n_dim = 100;

  graph::SparseNodeEmbedding input_embedding(n_node, n_dim);
//  graph::LoadSparseNodeEmbedding(feature_path, &input_embedding);

  graph::Graph graph(input_embedding);
  graph::LoadGraph(edge_path, &graph);
  graph.Build();

  input_embedding.RandomInit(graph.GetNodeSet());

  RunUnSupervisedGraphSage(graph);
}

void RunBlogCatalog() {
  const std::string edge_path = "../data/blogCatalog/bc.edge";

  int64_t n_node = 10312;
  int64_t n_dim = 100;

  graph::SparseNodeEmbedding input_embedding(n_node, n_dim);
//  graph::LoadSparseNodeEmbedding(feature_path, &input_embedding);

  graph::Graph graph(input_embedding);
  graph::LoadGraph(edge_path, &graph);
  graph.Build();

  std::cout << graph.GetNumNode() << std::endl;
  std::cout << graph.GetNumEdge() << std::endl;

  input_embedding.RandomInit(graph.GetNodeSet());

  RunUnSupervisedGraphSage(graph);
}

//UnSupervisedGraphsage train(const AdjList& adj,
//    const Nodes& nodes, const Edges& edges,
//    const int batch_size,
//    const int hidden_dim,
//    const int n_epoch,
//    const int n_node,
//    const int n_negative = 3) {
//
//  auto dataset = EdgeDataset(edges.srcs, edges.dsts);
//  auto sampler = RandomSampler(dataset.size().value());
//  auto options = torch::data::DataLoaderOptions(batch_size);
//  auto loader  = torch::data::make_data_loader(dataset, options, sampler);
//
//  int n_feature = static_cast<int>(nodes.features.size(1));
//  UnSupervisedGraphsage net(n_feature, hidden_dim);
//  SGD optim(net.parameters(), 0.1);
//
//  for (int epoch = 0; epoch < n_epoch; epoch ++) {
//
//    int batch_cnt = 0;
//    for (auto batch : *loader) {
//      auto start = std::chrono::system_clock::now();
//      optim.zero_grad();
//
//      auto srcs = batch[0][0];
//      auto dsts = batch[0][1];
//
//      int src = srcs.item().toInt();
//      int dst = dsts.item().toInt();
//      std::cout << "src " << src << " degree " << adj.degree(src) << std::endl;
//      std::cout << "dst " << dst << " degree " << adj.degree(dst) << std::endl;
//
//      int batch_size = srcs.size(0);
////      auto negs = negative_sampling(adj, srcs, n_negative, n_node).view({batch_size * n_negative});
//
//      std::cout << "forward src" << std::endl;
//
//      auto src_output = net.forward(srcs, nodes.features, nodes.node_to_index, adj);
//      std::cout << "forward dst" << std::endl;
//      auto dst_output = net.forward(dsts, nodes.features, nodes.node_to_index, adj);
////      std::cout << "forward neg" << std::endl;
////      auto neg_output = net.forward(negs, nodes.features, nodes.node_to_index, adj).view({batch_size, n_negative, hidden_dim});
//
//      std::cout << "pariwise_loss" << std::endl;
//      auto loss = net.PairwiseLoss(src_output, dst_output);
//      std::cout << "backward" << std::endl;
//      loss.backward();
//
//      std::cout << "step" << std::endl;
//      optim.step();
//      auto end = std::chrono::system_clock::now();
//      std::chrono::duration<double> cost = end - start;
//
//      std::cout << " epoch=" << epoch
//                << " batch=" << batch_cnt
//                << " batch_size=" << batch_size
//                << " loss=" << loss.item()
//                << " time=" << cost.count() << "s"
//                << std::endl;
//      batch_cnt ++;
//    }
//  }
//  return net;
//}
//
//void run_cora() {
//  std::string edge_path = "../data/cora/cora.adjs";
//  std::string node_path = "../data/cora/cora.content.id";
//  AdjList adj;
//  Nodes nodes;
//  Edges edges;
//  int n_node = 2708;
////  int n_features = 1433;
//  int n_features = 256;
//  load_edges(edge_path, &adj);
//  load_edges(edge_path, &edges);
////  load_features(node_path, &nodes, n_features, n_node);
//  random_features(node_path, &nodes, n_features, n_node);
//  int batch_size = 1;
//  int dim = 128;
//  auto graphsage = train(adj, nodes, edges, batch_size, dim, 10, n_node);
//  graphsage.save("unsupervised_cora", nodes, adj);
//}
//
//void run_blogcatalog() {
//  std::string edge_path = "../data/blogCatalog/bc_adjlist.txt";
//
//  AdjList adj;
//  Nodes nodes;
//  Edges edges;
//  int n_node = 10312;
//  int n_features = 256;
//
//  std::cout << "loading adjs" << std::endl;
//  load_edges(edge_path, &adj);
//  std::cout << "loading edges" << std::endl;
//  load_edges(edge_path, &edges);
//
//  std::cout << "random features" << std::endl;
//  random_features(edge_path, &nodes, n_features, n_node);
//  int batch_size = 1;
//  int dim = 128;
//
//  auto graphsage = train(adj, nodes, edges, batch_size, dim, 10, n_node);
//  graphsage.save("unsupervised_blog", nodes, adj);
//}



int main() {
//  RunCora();
  RunBlogCatalog();
  return 0;
}