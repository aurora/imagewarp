/**
 * Imagewarp.
 *
 * @copyright   copyright (c) 2018-present by Harald Lapp
 * @author      Harald Lapp <harald@octris.org>
 */

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <numeric>

using namespace cv;
using namespace std;
using json = nlohmann::json;

class ImageWarp {
    public:
        ImageWarp(const String& filename);
        void setForeground(const String& filename);
        void addImage(const String& filename, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y);
        void save(const String& filename);
    private:
        Mat bg_image;
        Mat fg_image;
        bool has_foreground = false;
        void applyLayer(Mat layer_image);
};

/**
 * Constructor defines background image of the scene.
 */
ImageWarp::ImageWarp(const String& filename)
{
    bg_image = imread(filename, IMREAD_COLOR);
}

/**
 * Set the foreground image of the scene.
 */
void ImageWarp::setForeground(const String& filename)
{
    fg_image = imread(filename, IMREAD_COLOR);
    has_foreground = true;
}

/**
 * Add image to the scene at specified points.
 */
void ImageWarp::addImage(const String& filename, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y)
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
    Mat warped_image;

    warpPerspective(img, warped_image, H, bg_image.size());

    applyLayer(warped_image);
}

/**
 * Merge image to background image.
 */
void ImageWarp::applyLayer(Mat layer_image)
{
    Mat gray, gray_inv, layer1_image, layer2_image;

    cvtColor(layer_image, gray, COLOR_BGR2RGB);
    threshold(gray, gray, 0, 255, THRESH_BINARY);
    bitwise_not(gray, gray_inv);

    bg_image.copyTo(layer1_image, gray_inv);
    layer_image.copyTo(layer2_image, gray);

    bg_image = layer1_image + layer2_image;
}

/**
 * Save created image into file.
 */
void ImageWarp::save(const String& filename)
{
    if (has_foreground) {
        applyLayer(fg_image);
    }

    imwrite(filename, bg_image);
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        cout << "usage: imagewarp <scene-file> <output-file> <place1-file>\n";
        cout << "             [<place2-file> ...]\n";
        cout << "\n";
        cout << "The scene-file needs to be a valid json configuration file. All other\n";
        cout << "files are image files. To omit a place specify '-' as place filename.\n";
        cout << "Take care: existing output files will be overwritten without warning!\n";
        cout << "\n";

        return 1;
    }

    std::ifstream scene_file(argv[1]);
    json j;
    scene_file >> j;

    if (j["background"].empty()) {
        cout << "You specified a scene-file, that does not contain a background image.\n";

        return 1;
    }

    if (!haveImageReader(j["background"].get<std::string>())) {
        cout << "The specified background image cannot be read.\n";

        return 1;
    }

    ImageWarp ImageWarp(j["background"].get<std::string>());

    if (!j["foreground"].empty()) {
        if (!haveImageReader(j["foreground"].get<std::string>())) {
            cout << "The specified foreground image cannot be read.\n";

            return 1;
        }

        ImageWarp.setForeground(j["foreground"].get<std::string>());
    }

    for (int i = 3; i < argc; ++i) {
        if (strcmp(argv[i], "-") == 0) {
            continue;
        }

        if (!haveImageReader(argv[i])) {
            cout << "The specified image cannot be read " << argv[i] << ".\n";

            return 1;
        }

        if (!j["places"][i - 3].empty()) {
            ImageWarp.addImage(
                argv[i],
                j["places"][i - 3]["p1"]["x"].get<float>(), j["places"][i - 3]["p1"]["y"].get<float>(),
                j["places"][i - 3]["p2"]["x"].get<float>(), j["places"][i - 3]["p2"]["y"].get<float>(),
                j["places"][i - 3]["p3"]["x"].get<float>(), j["places"][i - 3]["p3"]["y"].get<float>(),
                j["places"][i - 3]["p4"]["x"].get<float>(), j["places"][i - 3]["p4"]["y"].get<float>()
            );
        }
    }

    ImageWarp.save(argv[2]);

    return 0;
}
