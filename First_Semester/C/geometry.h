#include <algorithm>
#include <complex>
#include <cstring>
#include <utility>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

/// DECLARATIONS ///

struct Point;
class Line;
class Shape;
class Polygon;
class Ellipse;
class Circle;
class Rectangle;
class Triangle;
struct Vector;

struct Point {
  double x;
  double y;

  Point(double x, double y);
  Point(): Point(0, 0) {}
  bool operator==(const Point& other) const;

  void rotate(const Point& center, double angle);
  void reflect(const Point& center);
  void reflect_line(const Line& axis);
  void homotety(const Point& center, double coefficient);
};

class Line {
 public:
  double a = 1;
  double b = 0;
  double c = 0;
 
  Line(const Point& first, const Point& second);
  Line(double slope, double shift);
  Line(const Point& point, double slope);

  bool operator==(const Line& other) const;

 private:
  void normalize();
};

class Shape {
 public:
  virtual double perimeter() const = 0;
  virtual double area() const = 0;
  virtual bool operator==(const Shape& other) const;
  virtual bool isEqual(const Shape& other) const = 0;
  virtual bool isCongruentTo(const Shape& other) const = 0;
  virtual bool isSimilarTo(const Shape& other) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;
  virtual void rotate(const Point& center, double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;
  virtual void scale(const Point& center, double coefficient) = 0;

  virtual ~Shape() = default;
};

class Polygon: public Shape {
 public:
  std::vector<Point> points;

 private:
  double similarityCoefficient(const Shape& other) const;
  static double similarityHelping(const std::vector<Point>& copy_points, size_t points_size, const Polygon& other);
  static bool isEqualHelping(const Polygon& other, std::vector<Point>& local_copy);
 
 public:
  Polygon() = default;
  explicit Polygon(const std::vector<Point>& points);
  template <typename... Args>
  explicit Polygon(Args... args): points({args...}) {}

  bool isEqual(const Shape& sh_other) const override;

  double perimeter() const override;
  double area() const override;
  bool isCongruentTo(const Shape& other) const override;
  bool isSimilarTo(const Shape& other) const override;
  bool containsPoint(const Point& point) const override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;
  void scale(const Point& center, double coefficient) override;

  const std::vector<Point>& getVertices() const;
  size_t verticesCount() const;
  bool isConvex() const;
};

class Ellipse: public Shape {
 public:
  Point focus1;
  Point focus2;
  double sum;
 
  Ellipse(const Point& first, const Point& second, double sum);

  bool isEqual(const Shape& sh_other) const override;

  double perimeter() const override;
  double area() const override;
  bool isCongruentTo(const Shape& other) const override;
  bool isSimilarTo(const Shape& other) const override;
  bool containsPoint(const Point& point) const override;
  void rotate(const Point& center, double angle) override;
  void reflect(const Point& center) override;
  void reflect(const Line& axis) override;
  void scale(const Point& center, double coefficient) override;

  std::pair<Point,Point> focuses() const;
  std::pair<Line, Line> directrices() const;
  double eccentricity() const;
  Point center() const;

 protected:
  double getA_() const;
  double getB_() const;
};

class Circle: public Ellipse {
 public:
  Circle(const Point& center, double rad);

  double radius() const;
};

class Rectangle: public Polygon {
 public:
  Rectangle(const Point& first, const Point& second, double ratio);

  Point center() const;
  std::pair<Line, Line> diagonals() const;

  double area() const override;
};

class Square: public Rectangle {
 public:
  Square(const Point& first, const Point& second);

  double area() const final;

  Circle circumscribedCircle() const;
  Circle inscribedCircle() const;
};

class Triangle: public Polygon {
 public:
  Triangle(const Point& first, const Point& second, const Point& third);

  Circle circumscribedCircle() const;
  Circle inscribedCircle() const;
  Point centroid() const;
  Point orthocenter() const;
  Line EulerLine() const;
  Circle ninePointsCircle() const;
};

/// ADDITIONAL ///

struct Vector {
  Point coords;

  Vector(const Point& start, const Point& end);
  Vector(double x, double y);
  Vector(): Vector(0, 0) {}
  Vector(const Point& end);

  double length();
  double angle(const Vector& other);
  double dotProduct(const Vector& other);
  double crossProduct(const Vector& other);
};

namespace Additional {
double dist(const Point& first_point, const Point& second_point) {
  return Vector(first_point, second_point).length();
}

int sign(double a) {
  static constexpr double PRECISION = 1e-6;
  if (std::fabs(a) < PRECISION) {
    return 0;
  }
  if (a > 0) {
    return 1;
  }
  return -1;
}

bool equal(double first_double, double second_double) {
  return sign(first_double - second_double) == 0;
}

bool on_segment(const Point& point, const Point& point_a, const Point& point_b) {
  Vector vec_a_p = {point_a, point};
  Vector vec_p_b = {point, point_b};
  return equal(vec_a_p.angle(vec_p_b), 0);
}
}

/// VECTOR ///

Vector::Vector(const Point& start, const Point& end)
    : coords({end.x - start.x, end.y - start.y}) {}
  
Vector::Vector(double x, double y)
    : coords({x, y}) {}

Vector::Vector(const Point& end)
    :coords(end) {}

double Vector::length() {
  return std::sqrt(dotProduct(*this));
}

double Vector::angle(const Vector& other) {
  return std::atan2(crossProduct(other), dotProduct(other));
}

double Vector::dotProduct(const Vector& other) {
  return coords.x * other.coords.x + coords.y * other.coords.y;
}

double Vector::crossProduct(const Vector& other) {
  return coords.x * other.coords.y - coords.y * other.coords.x;
}

/// SHAPE ///

bool Shape::operator==(const Shape& other) const {
  return isEqual(other);
}

/// POINT ///

Point::Point(double x_coord, double y_coord): x(x_coord), y(y_coord) {}

bool Point::operator==(const Point& other) const {
  return Additional::equal(x, other.x) && Additional::equal(y, other.y);
}

void Point::rotate(const Point& center, double angle) {
  angle *= M_PI / 180;
  double shifted_x = x - center.x;
  double shifted_y = y - center.y;
  double new_x = shifted_x * cos(angle) - shifted_y * sin(angle);
  double new_y = shifted_x * sin(angle) + shifted_y * cos(angle);
  x = new_x + center.x;
  y = new_y + center.y;
}

void Point::reflect(const Point& other) {
  x = 2 * other.x - x;
  y = 2 * other.y - y;
}

void Point::reflect_line(const Line& axis) {
  double coef = (x * axis.a + y * axis.b + axis.c) / (axis.a * axis.a + axis.b * axis.b);
  x -= 2 * coef * axis.a;
  y -= 2 * coef * axis.b;
}

void Point::homotety(const Point& center, double coefficient) {
  x = center.x + coefficient * (x - center.x);
  y = center.y + coefficient * (y - center.y);
}

/// LINE ///

void Line::normalize() {
  if (!Additional::equal(a, 0)) {
    b /= a;
    c /= a;
    a = 1;
  } else if (b != 0) {
    a /= b;
    c /= b;
    b = 1;
  } else if (c != 0) {
    a /= c;
    b /= c;
    c = 1;
  }
}

Line::Line(const Point& first, const Point& second)
    : a(second.y - first.y)
    , b(first.x - second.x)
    , c(second.x * first.y - first.x * second.y) {
  normalize();
}

Line::Line(double slope, double shift)
    : a(slope)
    , b(-1)
    , c(shift) {
  normalize();
}

Line::Line(const Point& point, double slope)
    : a(slope)
    , b(-1)
    , c(point.y - slope * point.x) {
  normalize();
}

bool Line::operator==(const Line& other) const {
  return Additional::equal(a, other.a) && Additional::equal(b, other.b) && Additional::equal(c, other.c);
}

/// POLYGON ///

Polygon::Polygon(const std::vector<Point>& points): points(points) {}

bool Polygon::isEqualHelping(const Polygon& other, std::vector<Point>& local_copy) {
  if (local_copy.size() != other.points.size()) {
    return false;
  }
  for (size_t i = 0; i < local_copy.size(); ++i) {
    if (local_copy[i] == other.points[0]) {
      std::rotate(local_copy.begin(), local_copy.begin() + i, local_copy.end());
      return true;
    }
  }
  return false;
  
}

bool Polygon::isEqual(const Shape& sh_other) const {
  const Polygon* pother = dynamic_cast<const Polygon*>(&sh_other);
  if (pother == nullptr) {
    return false;
  }
  const Polygon& other = *pother;
  std::vector<Point> local_copy = points;
  if (!isEqualHelping(other, local_copy)) {
    return false;
  }
  if (local_copy == other.points) {
    return true;
  }
  std::reverse(local_copy.begin(), local_copy.end());
  if (!isEqualHelping(other, local_copy)) {
    return false;
  }
  return local_copy == other.points;
}

double Polygon::perimeter() const {
  double ans = 0;
  for (size_t i = 0; i < points.size() - 1; ++i) {
    ans += Additional::dist(points[i], points[i + 1]);
  }
  if (points.size() > 2) {
    ans += Additional::dist(points[0], points.back());
  }
  return ans;
}

double Polygon::area() const {
  if (points.size() < 3) {
    return 0;
  }
  double ans = 0;
  for (size_t i = 0; i < points.size() - 1; ++i) {
    ans += Vector(points[i]).crossProduct(Vector(points[i + 1])); 
  }
  ans += Vector(points.back()).crossProduct(Vector(points[0]));
  ans = fabs(ans) / 2;
  return ans;
}

bool Polygon::isCongruentTo(const Shape& sh_other) const {
  return Additional::equal(similarityCoefficient(sh_other), 1);
}

bool Polygon::isSimilarTo(const Shape& sh_other) const {
  return similarityCoefficient(sh_other) >= 0;
}

double Polygon::similarityCoefficient(const Shape& sh_other) const {
  const Polygon* pother = dynamic_cast<const Polygon*>(&sh_other);
  if (pother == nullptr) {
    return -1;
  }
  const Polygon& other = *pother;
  size_t points_size = points.size();
  if (points_size != other.points.size()) {
    return -1;
  }
  if (points_size == 1) {
    return 1;
  }
  if (points_size == 2) {
    return Additional::dist(points[0], points[1]) / Additional::dist(other.points[0], other.points[1]);
  }
  std::vector<Point> copy_points = points;
  double ab_ratio = similarityHelping(copy_points, points_size, other);
  if (ab_ratio != -1) {
    return ab_ratio;
  }
  std::reverse(copy_points.begin(), copy_points.end());
  return similarityHelping(copy_points, points_size, other);
}

double Polygon::similarityHelping(const std::vector<Point>& copy_points, size_t points_size, const Polygon& other) {
  for (size_t j = 0; j < points_size; ++j) {
    bool find = true;
    size_t first = 0;
    size_t second = j;
    double ab_ratio;
    double bc_ratio;
    for (size_t it = 0; it < points_size; ++it) {
      Vector AB(copy_points[first], copy_points[(first + 1) % points_size]);
      Vector BC(copy_points[(first + 1) % points_size], copy_points[(first + 2) % points_size]);
      Vector other_AB(other.points[second], other.points[(second + 1) % points_size]);
      Vector other_BC(other.points[(second + 1) % points_size], other.points[(second + 2) % points_size]);
      ab_ratio = AB.length() / other_AB.length();
      bc_ratio = BC.length() / other_BC.length();
      if ((!Additional::equal(ab_ratio, bc_ratio)) || !Additional::equal(fabs(AB.angle(BC)), fabs(other_AB.angle(other_BC)))) {
        find = false;
        break;
      }
      first = (first + 1) % points_size;
      second = (second + 1) % points_size;
    }
    if (find) {
      return ab_ratio;
    }
  }
  return -1;
}

bool Polygon::containsPoint(const Point& point) const {
  size_t points_size = points.size();
  for (size_t i = 0; i < points_size; ++i) {
    if (Additional::on_segment(point, points[i], points[(i + 1) % points_size])) {
      return true;
    }
  }
  double angle = 0;
  for (size_t i = 0; i < points_size; ++i) {
    Vector vec_p_a = {point, points[i]};
    Vector vec_p_b = {point, points[(i + 1) % points_size]};
    angle += vec_p_a.angle(vec_p_b);
  }
  return !Additional::equal(angle, 0);
}

void Polygon::rotate(const Point& center, double angle) {
  for (Point& point : points) {
    point.rotate(center, angle);
  }
}

void Polygon::reflect(const Point& center) {
  for (Point& point : points) {
    point.reflect(center);
  }
}

void Polygon::reflect(const Line& axis) {
  for (Point& point : points) {
    point.reflect_line(axis);
  }
}

void Polygon::scale(const Point& center, double coefficient) {
  for (Point& point : points) {
    point.homotety(center, coefficient);
  }
}

const std::vector<Point>& Polygon::getVertices() const {
  return points;
}

size_t Polygon::verticesCount() const {
  return points.size();
}

bool Polygon::isConvex() const {
  size_t points_size = points.size();
  for (size_t i = 0; i < points_size; ++i) {
    Vector vec_b_a = {points[(i + 1) % points_size], points[i % points_size]};
    Vector vec_b_c = {points[(i + 1) % points_size], points[(i + 2) % points_size]};
    Vector vec_c_b = {points[(i + 2) % points_size], points[(i + 1) % points_size]};
    Vector vec_c_d = {points[(i + 2) % points_size], points[(i + 3) % points_size]};

    if (Additional::sign(vec_b_a.angle(vec_b_c)) != Additional::sign(vec_c_b.angle(vec_c_d))) {
      return false;
    }
  }
  return true;
}

/// ELLIPSE ///

double Ellipse::getA_() const {
  return sum / 2;
}

double Ellipse::getB_() const {
  double ellipse_a = sum / 2;
  double ellipse_c = Additional::dist(focus1, focus2) / 2;
  return std::sqrt(ellipse_a * ellipse_a - ellipse_c * ellipse_c);
}

bool Ellipse::isEqual(const Shape& sh_other) const {
  const Ellipse* pother = dynamic_cast<const Ellipse*>(&sh_other);
  if (pother == nullptr) {
    return false;
  }
  const Ellipse& other = *pother;
  return Additional::equal(sum, other.sum) && (
      (focus1 == other.focus1 && focus2 == other.focus2)
    || (focus1 == other.focus2 && focus2 == other.focus1)
  );
}

Ellipse::Ellipse(const Point& first, const Point& second, double sum)
    : focus1(first)
    , focus2(second)
    , sum(sum) {}

double Ellipse::perimeter() const {
  double ellipse_a = getA_(); // большая полуось
  double ellipse_b = getB_(); // малая полуось
  double coef_h = std::pow((ellipse_a - ellipse_b) / (ellipse_a + ellipse_b), 2);
  double ans = M_PI * (ellipse_a + ellipse_b) * (1 + (3 * coef_h) / (10 + std::sqrt(4 - 3 * coef_h)));
  return ans;
}

double Ellipse::area() const {
  double ellipse_a = getA_(); // большая полуось
  double ellipse_b = getB_(); // малая полуось
  return M_PI * ellipse_a * ellipse_b;
}

bool Ellipse::isCongruentTo(const Shape& sh_other) const {
  const Ellipse* pother = dynamic_cast<const Ellipse*>(&sh_other);
  if (pother == nullptr) {
    return false;
  }
  const Ellipse& other = *pother;
  return Additional::equal(getA_(), other.getA_()) && Additional::equal(getB_(), other.getB_());
}

bool Ellipse::isSimilarTo(const Shape& sh_other) const {
  const Ellipse* pother = dynamic_cast<const Ellipse*>(&sh_other);
  if (pother == nullptr) {
    return false;
  }
  const Ellipse& other = *pother;
  double ratio_big_half_ax = getA_() / other.getA_();
  double ratio_small_half_ax = getB_() / other.getB_();
  return Additional::equal(ratio_big_half_ax, ratio_small_half_ax);
}

bool Ellipse::containsPoint(const Point& point) const {
  return (Additional::dist(point, focus1) + Additional::dist(point, focus2) < sum) 
    || Additional::equal(Additional::dist(point, focus1) + Additional::dist(point, focus2), sum);
}

void Ellipse::rotate(const Point& center, double angle) {
  focus1.rotate(center, angle);
  focus2.rotate(center, angle);
}

void Ellipse::reflect(const Point& center) {
  focus1.reflect(center);
  focus2.reflect(center);
}

void Ellipse::reflect(const Line& axis) {
  focus1.reflect_line(axis);
  focus2.reflect_line(axis);
}

void Ellipse::scale(const Point& center, double coefficient) {
  focus1.homotety(center, coefficient);
  focus2.homotety(center, coefficient);
  sum *= coefficient;
}

std::pair<Point, Point> Ellipse::focuses() const {
  return std::make_pair(focus1, focus2);
}

std::pair<Line, Line> Ellipse::directrices() const {
  Point delta = {
    (focus1.x - focus2.x) / Additional::dist(focus1, focus2),
    (focus1.y - focus2.y) / Additional::dist(focus1, focus2)
  };
  double slope = delta.x / delta.y;
  Point first = {
    (focus1.x + focus2.x) / 2 + delta.x * sum / eccentricity(),
    (focus1.y + focus2.y) / 2 + delta.y * sum / eccentricity()
  };
  Point second = {
    (focus1.x + focus2.x) / 2 - delta.x * sum / eccentricity(),
    (focus1.y + focus2.y) / 2 - delta.y * sum / eccentricity()
  };
  return {Line(first, slope), Line(second, slope)};
}

double Ellipse::eccentricity() const {
  return (Additional::dist(focus1, focus2) / 2) / getA_();
}

Point Ellipse::center() const {
  Point center = {(focus1.x + focus2.x) / 2, (focus1.y + focus2.y) / 2};
  return center;
}

/// CIRCLE ///

Circle::Circle(const Point& center, double rad)
    : Ellipse(center, center, 2 * rad) {}

double Circle::radius() const {
  return sum / 2;
}

/// RECTANGLE ///

Rectangle::Rectangle(const Point& first, const Point& second, double ratio) {
  Point center = {(first.x + second.x) / 2, (first.y + second.y) / 2};
  if (ratio > 1) {
    ratio = 1 / ratio;
  }
  double alpha = ((first.x < second.x) || (Additional::equal(first.x, second.x) && first.y < second.y) ? -1 : 1) * 90 / asin(1.0) * asin(ratio);
  Polygon polygon(first, second);
  polygon.rotate(center, alpha);
  points = {polygon.getVertices()[0], second, polygon.getVertices()[1], first};
}

Point Rectangle::center() const {
  return Point{(points[0].x + points[2].x) / 2, (points[0].y + points[2].y) / 2};
}

std::pair<Line, Line> Rectangle::diagonals() const {
    return std::make_pair<Line, Line>(Line(points[0], points[2]), Line(points[1], points[3]));
}

double Rectangle::area() const {
  return Vector(points[0], points[1]).length() * Vector(points[1], points[2]).length();
}

/// SQUARE ///

Square::Square(const Point& first, const Point& second)
    : Rectangle(first, second, 1) {}

Circle Square::circumscribedCircle() const {
  Circle ans = Circle(center(), Additional::dist(points[0], points[2]) / 2);
  return ans;
}

Circle Square::inscribedCircle() const {
  Circle ans = Circle(center(), Additional::dist(points[0], points[1]) / 2);
  return ans;
}

double Square::area() const {
  double side_length = Vector(points[0], points[1]).length();
  return side_length * side_length;
}

/// TRIANGLE ///

Triangle::Triangle(const Point& first, const Point& second, const Point& third)
    : Polygon(first, second, third) {}

Circle Triangle::circumscribedCircle() const {
  const Point& point_a = points[0];
  const Point& point_b = points[1];
  const Point& point_c = points[2];

  double coef = 2 * (point_a.x * (point_b.y - point_c.y) + point_b.x * (point_c.y - point_a.y) + point_c.x * (point_a.y - point_b.y));

  Point center;
  Vector vec_a(point_a);
  Vector vec_b(point_b);
  Vector vec_c(point_c);
  center.x = ((vec_a.dotProduct(vec_a)) * (point_b.y - point_c.y) +
              (vec_b.dotProduct(vec_b)) * (point_c.y - point_a.y) +
              (vec_c.dotProduct(vec_c)) * (point_a.y - point_b.y)) / coef;
  center.y = ((vec_a.dotProduct(vec_a)) * (point_c.x - point_b.x) +
              (vec_b.dotProduct(vec_b)) * (point_a.x - point_c.x) +
              (vec_c.dotProduct(vec_c)) * (point_b.x - point_a.x)) / coef;
  double rad = Vector(center, point_a).length();
  return Circle(center, rad);
}

Circle Triangle::inscribedCircle() const {
  const Point& point_a = points[0];
  const Point& point_b = points[1];
  const Point& point_c = points[2];

  double bc_len = Additional::dist(point_b, point_c);
  double ac_len = Additional::dist(point_a, point_c);
  double ab_len = Additional::dist(point_a, point_b);

  double half_perimeter = (bc_len + ac_len + ab_len) / 2.0;
  double area = std::sqrt(half_perimeter * (half_perimeter - bc_len) * (half_perimeter - ac_len) * (half_perimeter - ab_len));

  double rad = area / half_perimeter;
  Point center;
  center.x = (bc_len * point_a.x + ac_len * point_b.x + ab_len * point_c.x) / (bc_len + ac_len + ab_len);
  center.y = (bc_len * point_a.y + ac_len * point_b.y + ab_len * point_c.y) / (bc_len + ac_len + ab_len);

  Circle ans(center, rad);
  return ans;
}

Point Triangle::centroid() const {
  Point centroid(
    (points[0].x + points[1].x + points[2].x) / 3, 
    (points[0].y + points[1].y + points[2].y) / 3
  );
  return centroid;
}

Point Triangle::orthocenter() const {
  const Point& point_a = points[0];
  const Point& point_b = points[1];
  const Point& point_c = points[2];

  double area = (point_a.x * (point_b.y - point_c.y) + point_b.x * (point_c.y - point_a.y) + point_c.x * (point_a.y - point_b.y)) / 2.0;
  if (area == 0) {
      throw std::logic_error("Degenerate triangle: orthocenter does not exist");
  }

  double slope_bc, slope_ac, slope_h1, slope_h2, shift_h1, shift_h2;

  if (point_b.x == point_c.x) {
      slope_h1 = 0;
      shift_h1 = point_a.y;
  } else {
      slope_bc = (point_c.y - point_b.y) / (point_c.x - point_b.x);
      slope_h1 = -1 / slope_bc;
      shift_h1 = point_a.y - slope_h1 * point_a.x;
  }

  if (point_a.x == point_c.x) {
      slope_h2 = 0;
      shift_h2 = point_b.y;
  } else {
      slope_ac = (point_c.y - point_a.y) / (point_c.x - point_a.x);
      slope_h2 = -1 / slope_ac;
      shift_h2 = point_b.y - slope_h2 * point_b.x;
  }

  Point orthocenter;
  if (slope_h1 == slope_h2) {
      throw std::logic_error("Invalid triangle: heights do not intersect");
  }
  if (std::isinf(slope_h1)) {
      orthocenter.x = point_a.x;
      orthocenter.y = slope_h2 * orthocenter.x + shift_h2;
  } else if (std::isinf(slope_h2)) {
      orthocenter.x = point_b.x;
      orthocenter.y = slope_h1 * orthocenter.x + shift_h1;
  } else {
      orthocenter.x = (shift_h2 - shift_h1) / (slope_h1 - slope_h2);
      orthocenter.y = slope_h1 * orthocenter.x + shift_h1;
  }

  return orthocenter;
}

Line Triangle::EulerLine() const {
  return Line(orthocenter(), centroid());
}

Circle Triangle::ninePointsCircle() const {
  Circle out_circle = circumscribedCircle();
  Point ort = orthocenter();

  Point center = {(out_circle.center().x + ort.x) / 2, (out_circle.center().y + ort.y) / 2};
  double rad = out_circle.radius() / 2;
  Circle ans(center, rad);

  return ans;
}