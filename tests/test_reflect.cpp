#include <nanobind/nb_reflect.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <cmath>
#include <string>
#include <vector>

namespace nb = nanobind;

namespace reflect_test {

enum class Enum { A, B, C };

struct Struct {
    int i;
    double d;
    std::string s;

    static constexpr int static_const = 42;
    static int static_mut;

    Struct() : i(0), d(0), s() { ++static_mut; }
    Struct(int i, double d) : i(i), d(d), s() { ++static_mut; }

    int get_i() const { return i; }
    void set_i(int v) { i = v; }
    double sum() const { return i + d; }

    double overloaded(double x) const { return d + x; }
    double overloaded(int x, int y) const { return d + x + y; }

    static int create_count() { return static_mut; }
};

int Struct::static_mut = 0;

struct Nested {
    std::string name;
    int value;
    std::vector<int> items;
    Struct inner;
};

class Mixed {
public:
    int pub_i;
    std::string pub_s;

    Mixed() : pub_i(0), pub_s(), priv_i(0), priv_d(0) {}

    int get_pub() const { return pub_i; }

private:
    int priv_i;
    double priv_d;

    void priv_method() {}

protected:
    int prot_i;
};

namespace ns {

struct A {
    int x;
    double y;
};

struct B {
    std::string s;
};

enum class E { X, Y, Z };

double free_fn(double a, double b) { return a + b; }
double free_fn(double a, double b, double c) { return a + b + c; }

} // namespace ns

} // namespace reflect_test

NB_MODULE(test_reflect_ext, m) {
    nb::reflect_<^^reflect_test>(m);
}
