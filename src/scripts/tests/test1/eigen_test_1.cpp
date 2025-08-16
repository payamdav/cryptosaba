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

    // Eigen::Map<MatrixXd, Eigen::AlignmentType::Unaligned, Eigen::Stride<0, 2>> map_x(reinterpret_cast<double*>(points.data()), points.size(), 1);
    // Eigen::Map<MatrixXd, Eigen::AlignmentType::Aligned, Eigen::Stride<0, 2>> map_x(reinterpret_cast<double*>(points.data()), points.size(), 1);
    // Eigen::Map<MatrixXd, Eigen::AlignmentType::Aligned, Eigen::Stride<0, 2>> map_x(&points[0].y, points.size(), 1);
    Eigen::Map<VectorXd, 0, Stride<0, 2>> map_x(&points[0].x, points.size());
    std::cout << map_x << std::endl;
}


int main() {
    // test_1();
    eigen_map_over_vector_struct_test();
    return 0;
}
