/**
 * @file relevance-judgements.cpp
 * @author Hussein Hazimeh
 * Built based on interactive-search.cpp which is written by Sean Massung
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "meta/corpus/document.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/all.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/util/printing.h"
#include "meta/util/time.h"

using namespace meta;

/**
 * @param path The path to the file to open
 * @return the text content of that file
 */
std::string get_content(const std::string& path) {
  std::ifstream in{path};
  std::string str{(std::istreambuf_iterator<char>(in)),
                  std::istreambuf_iterator<char>()};
  std::replace(str.begin(), str.end(), '\n', ' ');
  return str;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
    return 1;
  }

  // Turn on logging to std::cerr.
  logging::set_cerr_logging();

  // Create an inverted index using a splay cache. The arguments forwarded
  //  to make_index are the config file for the index and any parameters
  //  for the cache. In this case, we set the maximum number of nodes in
  //  the splay_cache to be 30000.
  auto config = cpptoml::parse_file(argv[1]);
  auto idx = index::make_index<index::splay_inverted_index>(*config, 30000);

  // Create a ranking class based on the config file.
  int token;
  try {
    std::default_random_engine generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(1, 5);
    token = distribution(generator);
  } catch (...) {
    token = 1;
  }

  std::unique_ptr<index::ranker> ranker;
  // Randomly select one of the following rankers
  switch (token) {
    case 1:
      ranker = make_unique<index::okapi_bm25>();
      break;
    case 2:
      ranker = make_unique<index::jelinek_mercer>();
      break;
    case 3:
      ranker = make_unique<index::pivoted_length>();
      break;
    case 4:
      ranker = make_unique<index::dirichlet_prior>();
      break;
    case 5:
      ranker = make_unique<index::absolute_discount>();
      break;
    default:
      ranker = make_unique<index::okapi_bm25>();
  }

  // Find the path prefix to each document so we can print out the contents.
  std::string prefix = *config->get_as<std::string>("prefix") + "/" +
                       *config->get_as<std::string>("dataset") + "/";

  std::cout << "Enter your query" << std::endl << std::endl;
  std::string text;
  // Open the file to save the relevance jugdements to
  std::ofstream qrels("Assignment1/task8.txt");
  if (!qrels.is_open()) {
    std::cout << "Problem writing the output to the system. Make sure the "
                 "script has enough writing privileges. Quiting..."
              << std::endl;
    return 0;
  }

  while (true) {
    std::cout << "> ";
    std::getline(std::cin, text);

    if (text.empty()) break;

    std::cout << "Enter a description of your query:  ";
    std::string description;
    std::getline(std::cin, description);

    corpus::document query;
    query.content(text);  // set the query's content to be user input

    // Use the ranker to score the query over the index.
    std::vector<index::search_result> ranking;
    auto time =
        common::time([&]() { ranking = ranker->score(*idx, query, 20); });

    std::cout << "Showing top 20 of results (" << time.count() << "ms)"
              << std::endl;

    for (size_t i = 0; i < ranking.size() && i < 20; ++i) {
      auto path = *idx->metadata(ranking[i].d_id).get<std::string>("name");
      std::cout << printing::make_bold(std::to_string(i + 1) + ". " + path)
                << std::endl;
    }
    std::cout << "Enter the numbers of the relevant pages separated by spaces: "
              << std::endl;
    int relevant_doc;
    bool success = 1;
    std::vector<int> relevant_docs;
    std::string line;
    std::getline(std::cin, line);
    if (line.find_first_not_of("0123456789 ") != std::string::npos) success = 0;

    std::istringstream linestream(line);

    while (linestream >> relevant_doc) {
      if (relevant_doc < 1 || relevant_doc > 20 || success == 0) {
        success = 0;
        break;
      } else
        relevant_docs.push_back(relevant_doc);
    }
    if (!success) {
      std::cout << "Error: The relevance judgements should be valid numbers "
                   "separated by spaces. Repeat your query"
                << std::endl;
    } else {
      if (relevant_docs.size() != 0)  // Do not allow empty judgements
      {
        qrels << text + '\n';
        qrels << description + '\n';
        for (size_t i = 0; i < relevant_docs.size(); i++)
          qrels << std::to_string(ranking[relevant_docs[i] - 1].d_id) + ' ';
        qrels << '\n';
      }
    }
    std::cout << "Enter another query to continue or a blank query to quit"
              << std::endl
              << std::endl;
  }
  qrels.close();
}
