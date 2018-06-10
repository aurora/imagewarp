#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <limits>
#include <numeric>

using namespace cv;
using namespace std;

class Scene {
    public:
        Scene(const String& filename);
        void setForeground(const String& filename);
        void addImage(const String& filename, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y);
        void writeScene(const String& filename);
    private:
        Mat bg_image;
        Mat fg_image;
        void applyLayer(Mat layer_image);
};

/**
 * Constructor defines background image of the scene.
 */
Scene::Scene(const String& filename)
{
    bg_image = imread(filename, IMREAD_COLOR);
}

/**
 * Set the foreground image of the scene.
 */
void Scene::setForeground(const String& filename)
{
    fg_image = imread(filename, IMREAD_COLOR);
}

/**
 * Add image to the scene at specified points.
 */
void Scene::addImage(const String& filename, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y)
{
    vector<Point2f> bg_points;
    vector<Point2f> img_points;

    bg_points.push_back(Point2f(p1x, p1y));
    bg_points.push_back(Point2f(p2x, p2y));
    bg_points.push_back(Point2f(p3x, p3y));
    bg_points.push_back(Point2f(p4x, p4y));

    Mat img = imread(filename, IMREAD_COLOR);

    img_points.push_back(Point2f(float(0), float(0)));
    img_points.push_back(Point2f(float(0), float(img.rows)));
    img_points.push_back(Point2f(float(img.cols), float(img.rows)));
    img_points.push_back(Point2f(float(img.cols), float(0)));

    Mat H = findHomography(img_points, bg_points, 0);
    Mat transformed_image;

    warpPerspective(img, transformed_image, H, bg_image.size());

    applyLayer(transformed_image);
}

/**
 * Merge image to background image.
 */
void Scene::applyLayer(Mat layer_image)
{
    Mat gray, gray_inv, layer1_image, layer2_image;

    cvtColor(layer_image, gray, CV_BGR2GRAY);
    threshold(gray, gray, 0, 255, CV_THRESH_BINARY);
    bitwise_not(gray, gray_inv);

    bg_image.copyTo(layer1_image, gray_inv);
    layer_image.copyTo(layer2_image, gray);

    bg_image = layer1_image + layer2_image;
}

/**
 * Write scene to file.
 */
void Scene::writeScene(const String& filename)
{
    imwrite(filename, bg_image);
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        cout << "usage: scene <bgimage> <insertimg> <outimage>";
        exit(1);
    }

    Scene scene(argv[1]);
    scene.addImage(argv[2], float(323), float(1120), float(323), float(1993), float(1491), float(1975), float(1491), float(1400));
    scene.writeScene(argv[3]);

    return 0;
}
