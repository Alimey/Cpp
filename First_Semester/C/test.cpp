#include "geometry.h"
#include "C:\Users\Alimey\Desktop\programming\C++\Pro\Egor\Geometry\geometry.h"
#include <random>

std::pair<std::vector<Polygon>, std::vector<Correct::Polygon>> GeneratePoligons(size_t how_many) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> int_dist(1, 10);
  std::uniform_int_distribution<> double_dist(1, 10);

  std::vector<Polygon> my;
  std::vector<Correct::Polygon> correct;
  for (size_t i = 0; i < how_many; ++i) {
    size_t n = int_dist(gen);
    std::vector<Point> my_verticies;
    std::vector<Correct::Point> correct_verticies;
    for (size_t j = 0; j < n; ++j) {
      double x = double_dist(gen);
      double y = double_dist(gen);
      Point my_vertice(x, y);
      Correct::Point correct_vertice(x, y);
      my_verticies.push_back(my_vertice);
      correct_verticies.push_back(correct_vertice);
    }
    Polygon my_add(my_verticies);
    Correct::Polygon correct_add(correct_verticies);
    my.push_back(my_add);
    correct.push_back(correct_add);
  }

  return std::make_pair(my, correct);
}

namespace Equality {
  std::vector<bool> CheckMyEqual(std::vector<Polygon>& first, std::vector<Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i] == second[i]);
    }
    return ans;
  }

  std::vector<bool> CheckCorrectEqual(std::vector<Correct::Polygon>& first, std::vector<Correct::Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i] == second[i]);
    }
    return ans;
  }
}

namespace Similarity {
  std::vector<bool> CheckMySimilarity(std::vector<Polygon>& first, std::vector<Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i].isSimilarTo(second[i]));
    }
    return ans;
  }

  std::vector<bool> CheckCorrectSimilarity(std::vector<Correct::Polygon>& first, std::vector<Correct::Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i].isSimilarTo(second[i]));
    }
    return ans;
  }
}

namespace Congruency {
  std::vector<bool> CheckMyCongruency(std::vector<Polygon>& first, std::vector<Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i].isCongruentTo(second[i]));
    }
    return ans;
  }

  std::vector<bool> CheckCorrectCongruency(std::vector<Correct::Polygon>& first, std::vector<Correct::Polygon>& second) {
    std::vector<bool> ans;
    for (size_t i = 0; i < first.size(); ++i) {
      ans.push_back(first[i].isCongruentTo(second[i]));
    }
    return ans;
  }
}

void printMyPolygon(const Polygon& my) {
  std::cout << "MY POLYGON: \n";
  for (size_t i = 0; i < my.getVertices().size(); ++i) {
    std::cout << "(" << my.getVertices()[i].x << ", " << my.getVertices()[i].y << ") ";
  }
  std::cout << "\n";
}

void printCorrectPolygon(const Correct::Polygon& correct) {
  std::cout << "CORRECT POLYGON: \n";
  for (size_t i = 0; i < correct.getVertices().size(); ++i) {
    std::cout << "(" << correct.getVertices()[i].x << ", " << correct.getVertices()[i].y << ") ";
  }
  std::cout << "\n";
}

int main() {
  bool EQUALITY = false;
  bool CONGRUENCY = true;
  bool SIMILARITY = false;

  size_t how_many = 10000;

  /// EQUALITY ///
  if (EQUALITY) {
    using namespace Equality;
    auto first_pair = GeneratePoligons(how_many);
    auto second_pair = GeneratePoligons(how_many);
    auto my_first = first_pair.first;
    auto correct_first = first_pair.second;
    auto my_second = second_pair.first;
    auto correct_second = second_pair.second;
    std::cout << (CheckMyEqual(my_first, my_second) == CheckCorrectEqual(correct_first, correct_second));
  }
  /// SIMILARITY ///
  if (SIMILARITY) {
    using namespace Similarity;
    auto first_pair = GeneratePoligons(how_many);
    auto second_pair = GeneratePoligons(how_many);
    auto my_first = first_pair.first;
    auto correct_first = first_pair.second;
    auto my_second = second_pair.first;
    auto correct_second = second_pair.second;
    std::cout << (CheckMySimilarity(my_first, my_second) == CheckCorrectSimilarity(correct_first, correct_second));
  }
  /// CONGRUENCY ///
  if (CONGRUENCY) {
    using namespace Congruency;
    auto first_pair = GeneratePoligons(how_many);
    auto second_pair = GeneratePoligons(how_many);
    auto my_first = first_pair.first;
    auto correct_first = first_pair.second;
    auto my_second = second_pair.first;
    auto correct_second = second_pair.second;
    std::vector<bool> my_answers = CheckMyCongruency(my_first, my_second);
    std::vector<bool> correct_answers = CheckCorrectCongruency(correct_first, correct_second);
    for (size_t i = 0; i < my_answers.size(); ++i) {
      if (my_answers[i] != correct_answers[i]) {
        std::cout << my_answers[i] << " " << correct_answers[i] << "\n";
        printMyPolygon(my_first[i]);
        printMyPolygon(my_second[i]);
      }
    }
  }
}
