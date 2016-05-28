/**
 * @file ranking-experiment.cpp
 * @author Hussein Hazimeh
 * Built based on query-runner.cpp which is written by Sean Massung
 */

#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "meta/corpus/document.h"
#include "meta/index/eval/ir_eval.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/index/score_data.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/time.h"

using namespace meta;

// pl2_ranker is a class that implements the pl2 ranking function. It is derived
// from the base class ranker.
class pl2_ranker : public index::ranker {
 private:
  // c_ and lambda_ are the parameters of pl2
  float c_ = 7;
  float lambda_ = 0.1;

 public:
  const static util::string_view id;
  // Default Constructor
  pl2_ranker();
  // Constructor which can set parameter values
  pl2_ranker(float c, float lambda);
  pl2_ranker(std::istream& in);
  // Set parameters
  void set_param(float c, float lambda) {
    c_ = c;
    lambda_ = lambda;
  };
  // Calculates the score for a single matched term
  float score_one(const index::score_data&) override;
  // Saves this ranker, if used in a classifier
  void save(std::ostream& out) const override;
};
// Used to identify pl2_ranker in config.toml
const util::string_view pl2_ranker::id = "pl2";
pl2_ranker::pl2_ranker() {}
pl2_ranker::pl2_ranker(float c, float lambda) : c_{c}, lambda_{lambda} {}
pl2_ranker::pl2_ranker(std::istream& in)
    : c_{io::packed::read<float>(in)}, lambda_{io::packed::read<float>(in)} {}

void pl2_ranker::save(std::ostream& out) const {
  io::packed::write(out, id);
  io::packed::write(out, c_);
  io::packed::write(out, lambda_);
}

float pl2_ranker::score_one(const index::score_data& sd) {
  /**
   * This function is called for each matched term between the query and the
   * document. The function's argument is a struct that contains important
   * information about the matched term. For example, sd.doc_term_count gives
   * the # of occurrences of the term in the document.
  **/
  float doc_len = sd.idx.doc_size(sd.d_id);  // Current document length
  float avg_dl = sd.avg_dl;      // Average document length in the corpus
  float tf = sd.doc_term_count;  // Raw term count in the document
  float pi = 3.14;               // Use this for pi - Do NOT use other values
  float lambda = lambda_;        // pl2's parameter
  float c = c_;                  // pl2's parameter

  /**
   * You should implement the ranking function after this comment.
   * Use all the variables provided above (doc_len, avg_dl, tf, pi, lambda,
   * and c)
   * Use the function log2() to implement the logarithm
   * Use exp(1) to implement e
  **/

  return 0;  // Change 0 to the final score you calculated
}

void pl2_tune(const std::shared_ptr<index::dblru_inverted_index>& idx,
              std::vector<corpus::document>& allqueries, index::ir_eval& eval,
              float& c, float& lambda) {
  /**
   * This function prints the best MAP and the corresponding c and lambda by
   *  testing different combinations of the parameters.
   * idx is the inverted index
   * allqueries is a vector containing all the queries in moocs-queries.txt
   * eval is an instance of ir_eval used to calculate the MAP (makes use
   *  of qrel.txt)
  **/

  float cvalues[3] = {0.3, 0.6, 0.9};  // Different values for the parameter c
  float lambdavalues[8] = {
      0.000001, 0.00001, 0.0001, 0.001, 0.01,
      0.1,      1,       10};  // Different values for the parameter lambda
  double maxmap = 0;           // Stores the current maximum MAP value
  // Stores the current optimal c (i.e. c that achieves max MAP) - Ignore the
  // initial value
  float cmax = 7;
  // Stores the current optimal lambda - Ignore the initial value
  float lambdamax = 0.1;
  // creates a pointer to a pl2_ranker instance
  auto ranker = make_unique<pl2_ranker>();

  for (int i = 0; i < 3; i++)  // Loops over all c values
  {
    for (int j = 0; j < 8; j++)  // Loops over all lambda values
    {
      ranker->set_param(cvalues[i], lambdavalues[j]);  // Sets the parameters of
                                                       // ranker to the current
                                                       // values of c and lambda
      for (std::vector<corpus::document>::iterator query = allqueries.begin();
           query != allqueries.end();
           ++query)  // Iterates over all queries in allqueries
      {
        auto ranking = ranker->score(*idx, *query, 1000);  // Returns a ranked
                                                           // list of the top
                                                           // 1000 documents for
                                                           // the current query
        eval.avg_p(ranking, (*query).id(), 1000);  // eval.avg_p stores the
                                                   // value of average precision
                                                   // for the current query in
                                                   // the instance eval
      }
      if (eval.map() > maxmap)  // Updates maxmap, cmax, lambdamax if the
                                // current map, which is equal to eval.map(), is
                                // greater than maxmap
      {
        // You should only change the values of the following three assignments
        maxmap = 0;     // Change 0 to the correct value
        cmax = 0;       // Change 0 to the correct value
        lambdamax = 0;  // Change 0 to the correct value
      }

      eval.reset_stats();  // Deletes all the average precision values stored in
                           // eval to allow correct calculation of MAP for the
                           // next parameter combination
    }
  }
  std::cout << "Max MAP = " << maxmap << " achieved by "
            << "c = " << cmax << ", lambda = " << lambdamax
            << std::endl;  // Prints to the standard ouput
  c = cmax;                // Returns the best c value to the calling function
  lambda = lambdamax;  // Returns the best lambda value to the calling function
}

namespace meta {
namespace index {
template <>
std::unique_ptr<ranker> make_ranker<pl2_ranker>(
    const cpptoml::table& config)  // Used by pl2_ranker to read the parameters
                                   // c and lambda from config.toml - You can
                                   // ignore it
{
  return make_unique<pl2_ranker>(
      config.get_as<double>("c").value_or(7.0f),
      config.get_as<double>("lambda").value_or(0.1f));
}
}
}

int main(int argc, char* argv[]) {
  index::register_ranker<pl2_ranker>();  // Registers pl2_ranker so that you can
                                         // use it in config.toml

  if (argc != 2 && argc != 3) {
    std::cerr << "Usage:\t" << argv[0] << " configFile"
              << " (Task#)" << std::endl;
    return 1;
  }

  if (argc == 3 &&
      !(strcmp(argv[2], "task4") == 0 || strcmp(argv[2], "task5") == 0 ||
        strcmp(argv[2], "task6") == 0 || strcmp(argv[2], "task7") == 0)) {
    std::cout << "Invalid Usage" << std::endl;
    return 1;
  }

  // Log to standard error
  logging::set_cerr_logging();

  // Register additional analyzers
  parser::register_analyzers();
  sequence::register_analyzers();

  // Submission Specific
  std::ofstream submission;
  if (argc == 3) {
    submission.open("Assignment1/" + std::string(argv[2]) + ".txt");
    if (!submission.is_open()) {
      std::cout << "Problem writing the output to the system. Make sure the "
                   "script has enough writing privileges. Quiting..."
                << std::endl;
      return 0;
    }
  }
  // End of Submission Specific Code

  // Create an inverted index using a DBLRU cache. The arguments forwarded to
  //  make_index are the config file for the index and any parameters for the
  //  cache. In this case, we set the maximum hash table size for the
  //  dblru_cache to be 30000.
  auto config = cpptoml::parse_file(argv[1]);
  auto idx = index::make_index<index::dblru_inverted_index>(*config, 30000);

  // Create a ranking class based on the config file.
  auto group = config->get_table("ranker");
  if (!group)
    throw std::runtime_error{"\"ranker\" group needed in config file!"};
  auto ranker = index::make_ranker(*group);

  // Get the path to the file containing queries
  auto query_path = config->get_as<std::string>("querypath");
  if (!query_path)
    throw std::runtime_error{"config file needs a \"querypath\" parameter"};

  std::ifstream queries{*query_path + *config->get_as<std::string>("dataset") +
                        "-queries.txt"};

  // Create an instance of ir_eval to evaluate the MAP and Precision@10
  auto eval = index::ir_eval(*config);
  std::string content;
  if (argc == 3 &&
      strcmp(argv[2], "task7") == 0)  // For task 7, pl2_tune is called.
  {
    std::vector<corpus::document> allqueries;
    size_t i = 1;
    while (queries.good()) {
      std::getline(queries, content);
      corpus::document query{doc_id{i - 1}};
      query.content(content);
      allqueries.push_back(query);
      auto ranking = ranker->score(*idx, query, 1000);
      eval.avg_p(ranking, query.id(), 1000);
      i++;
    }
    float c = 7;
    float lambda = 0.1;
    pl2_tune(idx, allqueries, eval, c, lambda);
    submission << std::setprecision(5) << c << ' ';
    submission << std::setprecision(5) << lambda << '\n';
    submission.close();
    return 0;
  }

  // For the rest of the tasks, the top 10 documents and Precision@10 is printed
  // for each test query, and finally the MAP is printed.
  auto elapsed_seconds = common::time([&]() {
    size_t i = 1;
    while (queries.good()) {
      std::getline(queries, content);
      corpus::document query{doc_id{i - 1}};
      query.content(content);
      std::cout << "Ranking query " << i++ << ": " << content << std::endl;
      auto ranking = ranker->score(*idx, query, 1000);
      std::cout << "Precision@10 for this query: "
                << eval.precision(ranking, query.id(), 10) << std::endl;
      if (argc == 3) {
        submission << std::setprecision(5) << ranking[0].score << ' ';
      }
      eval.avg_p(ranking, query.id(), 1000);
      std::cout << "Showing top 10 of " << ranking.size() << " results."
                << std::endl;
      for (size_t i = 0; i < ranking.size() && i < 10; ++i) {
        auto path = *idx->metadata(ranking[i].d_id).get<std::string>("name");
        std::cout << (i + 1) << ". "
                  << " " << path << " " << ranking[i].score << std::endl;
      }
      std::cout << std::endl;
    }
  });
  std::cout << "The MAP for all the queries: " << eval.map() << std::endl;
  if (argc == 3) {
    submission << std::setprecision(5) << eval.map();
    submission.close();
  }
  std::cout << "Elapsed time: " << elapsed_seconds.count() << "ms" << std::endl;

  return 0;
}
