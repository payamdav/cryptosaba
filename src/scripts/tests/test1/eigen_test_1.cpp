#include <Eigen/Dense>
#include <iostream>
#include <vector>


using namespace Eigen;

int test_1() {
    MatrixXd m(2, 2);
    m(0, 0) = 3;
    m(1, 0) = 2.5;
    m(0, 1) = -1;
    m(1, 1) = m(1, 0) + m(0, 1);
    std::cout << m << std::endl;
    return 0;
}

void eigen_map_over_vector_struct_test() {
    struct Point {
        double x;
        double y;
    };

    std::vector<Point> points = {
        {1.0, 2.0},
        {3.0, 4.0},
        {5.0, 6.0}
    };

    // create a  1 * 3 Map of points that grabs only x from points - so it requires using stride with size of Point
    typedef Eigen::Stride<sizeof(Point)/sizeof(double), 2> PointStride;
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor>, 0, PointStride> map_x(
        reinterpret_cast<const double*>(&points[0].x),
        points.size(),
        1,
        PointStride(sizeof(Point)/sizeof(double), 1)
    );

    std::cout << map_x << std::endl;
}


int main() {
    // test_1();
    eigen_map_over_vector_struct_test();
    return 0;
}
