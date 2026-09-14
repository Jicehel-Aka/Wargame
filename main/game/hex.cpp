#include "hex.h"
#include <cmath>
#include <algorithm>

namespace wg {

// Conversion axiale -> cube pour calcul de distance simple
static void to_cube(const Hex& h, int* x, int* y, int* z) {
    *x = h.q;
    *z = h.r;
    *y = -*x - *z;
}

int hex_distance(const Hex& a, const Hex& b) {
    int ax, ay, az, bx, by, bz;
    to_cube(a, &ax, &ay, &az);
    to_cube(b, &bx, &by, &bz);
    return (std::abs(ax - bx) + std::abs(ay - by) + std::abs(az - bz)) / 2;
}

// Directions flat-top en axial (ordre : E, NE, NW, W, SW, SE)
static const Hex kDirs[6] = {
    {1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}
};

std::vector<Hex> hex_neighbors(const Hex& h) {
    std::vector<Hex> out;
    out.reserve(6);
    for (const auto& d : kDirs)
        out.push_back({h.q + d.q, h.r + d.r});
    return out;
}

std::vector<Hex> hex_range(const Hex& center, int range) {
    std::vector<Hex> out;
    for (int dq = -range; dq <= range; dq++) {
        int r1 = std::max(-range, -dq - range);
        int r2 = std::min(range, -dq + range);
        for (int dr = r1; dr <= r2; dr++)
            out.push_back({center.q + dq, center.r + dr});
    }
    return out;
}

std::vector<Hex> hex_ring(const Hex& center, int range) {
    if (range == 0) return { center };
    std::vector<Hex> out;
    Hex h = { center.q + kDirs[4].q * range, center.r + kDirs[4].r * range };
    for (int side = 0; side < 6; side++) {
        for (int step = 0; step < range; step++) {
            out.push_back(h);
            h = { h.q + kDirs[side].q, h.r + kDirs[side].r };
        }
    }
    return out;
}

void hex_to_pixel(const Hex& h, float size, float origin_x, float origin_y, float* out_x, float* out_y) {
    // flat-top : largeur horizontale = 1.5*size par colonne, hauteur = sqrt(3)*size, lignes décalées
    float x = size * (1.5f * h.q);
    float y = size * (sqrtf(3.0f) * (h.r + h.q * 0.5f));
    *out_x = origin_x + x;
    *out_y = origin_y + y;
}

Hex pixel_to_hex(float px, float py, float size, float origin_x, float origin_y) {
    float x = (px - origin_x) / size;
    float y = (py - origin_y) / size;
    float q = (2.0f / 3.0f) * x;
    float r = (-1.0f / 3.0f) * x + (sqrtf(3.0f) / 3.0f) * y;

    // round axial cube
    float cx = q, cz = r, cy = -cx - cz;
    float rx = roundf(cx), ry = roundf(cy), rz = roundf(cz);
    float dx = fabsf(rx - cx), dy = fabsf(ry - cy), dz = fabsf(rz - cz);
    if (dx > dy && dx > dz) rx = -ry - rz;
    else if (dy > dz) ry = -rx - rz;
    else rz = -rx - ry;

    return { (int)rx, (int)rz };
}

} // namespace wg
