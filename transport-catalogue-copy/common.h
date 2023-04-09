#pragma once

#include <string>
#include <vector>

struct Size {
    int width = 0, height = 0;
};

struct Point {
    int x = 0, y = 0;
};

using Image = std::vector<std::string>;

inline Size GetImageSize(const Image& image) {
    return {image.empty() ? 0 : static_cast<int>(image[0].size()), static_cast<int>(image.size())};
}

inline bool IsPointInEllipse(Point p, Size size) {
    return (p.x + 0.5) * (p.x + 0.5) / (size.width / 2.0) / (size.width / 2.0) + (p.y + 0.5) * (p.y + 0.5) / (size.height / 2.0) / (size.height / 2.0) <= 1;
}