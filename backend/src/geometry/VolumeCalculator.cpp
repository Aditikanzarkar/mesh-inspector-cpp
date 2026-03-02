#include "geometry/VolumeCalculator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/AABB.h"
#include "core/Vector3.h"


constexpr double kEpsilon = 1e-9;
constexpr double kRayBarycentricEpsilon = 1e-8;
constexpr std::size_t kMaxSamples = 800000;
constexpr std::size_t kMinSamplesPerAxis = 6;
constexpr std::size_t kMaxSamplesPerAxis = 140;

    struct OctreeNode {
    AABB bounds{};
    std::vector<std::size_t> triangleIndices;
    std::array<int, 8> children{-1, -1, -1, -1, -1, -1, -1, -1};
    };

    struct OctreeConfig {
        std::size_t maxDepth = 8;
        std::size_t leafTriangleLimit = 48;
        double minLeafExtent = 0.0;
    };

    OctreeConfig buildOctreeConfig(const AABB& bounds, std::size_t triangleCount) {
        const Vector3 ext = boundsExtent(bounds);
        const double longest = std::max({ext.x, ext.y, ext.z, kEpsilon});
        const double shortest = std::max(std::min({ext.x, ext.y, ext.z}), kEpsilon);
        const double aspect = longest / shortest;

        OctreeConfig cfg{};
        cfg.maxDepth =
            static_cast<std::size_t>(std::min(11.0, std::max(5.0, std::ceil(5.0 + std::log2(aspect + 1.0)))));
        cfg.leafTriangleLimit = static_cast<std::size_t>(std::max<std::size_t>(
            24, static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(triangleCount))))));
        cfg.minLeafExtent = longest / (10.0 + 4.0 * std::log2(aspect + 1.0));
        return cfg;
    }

    std::array<AABB, 8> makeChildrenBounds(const AABB& bounds) {
        const Vector3 center = (bounds.min + bounds.max) * 0.5;
        std::array<AABB, 8> children{};

        for (int i = 0; i < 8; ++i) {
            AABB child{};
            child.min = Vector3{
                (i & 1) ? center.x : bounds.min.x,
                (i & 2) ? center.y : bounds.min.y,
                (i & 4) ? center.z : bounds.min.z,
            };
            child.max = Vector3{
                (i & 1) ? bounds.max.x : center.x,
                (i & 2) ? bounds.max.y : center.y,
                (i & 4) ? bounds.max.z : center.z,
            };
            children[static_cast<std::size_t>(i)] = child;
        }

        return children;
    }

    void buildOctreeRecursive(
        std::vector<OctreeNode>& nodes,
        int nodeIndex,
        const std::vector<AABB>& triBounds,
        const OctreeConfig& cfg,
        std::size_t depth
    ) {
        const std::size_t idx = static_cast<std::size_t>(nodeIndex);
        const Vector3 ext = boundsExtent(nodes[idx].bounds);
        const double nodeLongest = std::max({ext.x, ext.y, ext.z});

        if (depth >= cfg.maxDepth || nodes[idx].triangleIndices.size() <= cfg.leafTriangleLimit ||
            nodeLongest <= cfg.minLeafExtent) {
            return;
        }

        const std::array<AABB, 8> childBounds = makeChildrenBounds(nodes[idx].bounds);
        std::array<std::vector<std::size_t>, 8> childTriangles;

        for (std::size_t triIndex : nodes[idx].triangleIndices) {
            for (int c = 0; c < 8; ++c) {
                if (overlaps(triBounds[triIndex], childBounds[static_cast<std::size_t>(c)])) {
                    childTriangles[static_cast<std::size_t>(c)].push_back(triIndex);
                }
            }
        }

        bool splitUseful = false;
        for (const auto& ct : childTriangles) {
            if (!ct.empty() && ct.size() < nodes[idx].triangleIndices.size()) {
                splitUseful = true;
                break;
            }
        }
        if (!splitUseful) {
            return;
        }

        for (int c = 0; c < 8; ++c) {
            if (childTriangles[static_cast<std::size_t>(c)].empty()) {
                continue;
            }
            OctreeNode child{};
            child.bounds = childBounds[static_cast<std::size_t>(c)];
            child.triangleIndices = std::move(childTriangles[static_cast<std::size_t>(c)]);
            const int childIndex = static_cast<int>(nodes.size());
            nodes.push_back(std::move(child));
            nodes[idx].children[static_cast<std::size_t>(c)] = childIndex;
        }

        nodes[idx].triangleIndices.clear();
        nodes[idx].triangleIndices.shrink_to_fit();

        for (int c = 0; c < 8; ++c) {
            const int childIndex = nodes[idx].children[static_cast<std::size_t>(c)];
            if (childIndex >= 0) {
                buildOctreeRecursive(nodes, childIndex, triBounds, cfg, depth + 1);
            }
        }
    }

    std::vector<OctreeNode> buildOctree(const std::vector<Triangle>& triangles, const AABB& bounds) {
        std::vector<AABB> triBounds;
        triBounds.reserve(triangles.size());
        for (const Triangle& tri : triangles) {
            triBounds.push_back(triangleBounds(tri));
        }

        OctreeNode root{};
        root.bounds = bounds;
        root.triangleIndices.resize(triangles.size());
        for (std::size_t i = 0; i < triangles.size(); ++i) {
            root.triangleIndices[i] = i;
        }

        std::vector<OctreeNode> nodes;
        nodes.reserve(triangles.size() / 4 + 1);
        nodes.push_back(std::move(root));

        const OctreeConfig cfg = buildOctreeConfig(bounds, triangles.size());
        buildOctreeRecursive(nodes, 0, triBounds, cfg, 0);
        return nodes;
    }

    void collectCandidateTriangles(
        const std::vector<OctreeNode>& octree,
        int nodeIndex,
        const Vector3& origin,
        const Vector3& rayDir,
        std::vector<std::size_t>& outTriangles
    ) {
        const OctreeNode& node = octree[static_cast<std::size_t>(nodeIndex)];
        if (!intersectRayAABB(origin, rayDir, node.bounds, kEpsilon)) {
            return;
        }

        bool hasChildren = false;
        for (int childIndex : node.children) {
            if (childIndex >= 0) {
                hasChildren = true;
                collectCandidateTriangles(octree, childIndex, origin, rayDir, outTriangles);
            }
        }

        if (!hasChildren) {
            outTriangles.insert(outTriangles.end(), node.triangleIndices.begin(), node.triangleIndices.end());
        }
    }

bool intersectRayTriangleStrict(
    const Vector3& origin,
    const Vector3& dir,
    const Triangle& tri,
    double& outT
) {
    const Vector3 edge1 = tri.v2 - tri.v1;
    const Vector3 edge2 = tri.v3 - tri.v1;
    const Vector3 pvec = cross(dir, edge2);
    const double det = dot(edge1, pvec);

    if (std::fabs(det) <= kEpsilon) {
        return false;
    }
    const double invDet = 1.0 / det;

    const Vector3 tvec = origin - tri.v1;
    const double u = dot(tvec, pvec) * invDet;
    if (u <= kRayBarycentricEpsilon || u >= (1.0 - kRayBarycentricEpsilon)) {
        return false;
    }

    const Vector3 qvec = cross(tvec, edge1);
    const double v = dot(dir, qvec) * invDet;
    if (v <= kRayBarycentricEpsilon || (u + v) >= (1.0 - kRayBarycentricEpsilon)) {
        return false;
    }

    const double t = dot(edge2, qvec) * invDet;
    if (t <= kEpsilon) {
        return false;
    }

    outT = t;
    return true;
}

std::size_t clampAxisSamples(std::size_t samples) {
    return std::min(std::max(samples, kMinSamplesPerAxis), kMaxSamplesPerAxis);
}

void enforceTotalSampleCap(std::size_t& nx, std::size_t& ny, std::size_t& nz) {
    while (nx * ny * nz > kMaxSamples) {
        if (nx >= ny && nx >= nz && nx > kMinSamplesPerAxis) {
            --nx;
            continue;
        }
        if (ny >= nx && ny >= nz && ny > kMinSamplesPerAxis) {
            --ny;
            continue;
        }
        if (nz > kMinSamplesPerAxis) {
            --nz;
            continue;
        }
        break;
    }
}

void computeGridResolution(
    const AABB& bounds,
    std::size_t& nx,
    std::size_t& ny,
    std::size_t& nz
) {
    const Vector3 ext = boundsExtent(bounds);
    const double longest = std::max({ext.x, ext.y, ext.z, kEpsilon});
    const double shortest = std::max(std::min({ext.x, ext.y, ext.z}), kEpsilon);
    const double aspect = longest / shortest;

    const double baseLongestSamples = 40.0 + 6.0 * std::log2(aspect + 1.0);
    nx = clampAxisSamples(static_cast<std::size_t>(std::ceil(baseLongestSamples * ext.x / longest)));
    ny = clampAxisSamples(static_cast<std::size_t>(std::ceil(baseLongestSamples * ext.y / longest)));
    nz = clampAxisSamples(static_cast<std::size_t>(std::ceil(baseLongestSamples * ext.z / longest)));

    enforceTotalSampleCap(nx, ny, nz);
}

bool pointInsideMesh(
    const Vector3& point,
    const std::vector<Triangle>& triangles,
    const std::vector<OctreeNode>& octree,
    const Vector3& rayDir,
    std::vector<std::uint32_t>& visitedStamp,
    std::uint32_t stamp
) {
    std::vector<std::size_t> candidates;
    candidates.reserve(64);
    collectCandidateTriangles(octree, 0, point, rayDir, candidates);

    int hitCount = 0;
    for (std::size_t triIndex : candidates) {
        if (visitedStamp[triIndex] == stamp) {
            continue;
        }
        visitedStamp[triIndex] = stamp;

        double t = 0.0;
        if (intersectRayTriangleStrict(point, rayDir, triangles[triIndex], t)) {
            ++hitCount;
        }
    }

    return (hitCount % 2) == 1;
}

double VolumeCalculator::estimateVolume(const std::vector<Triangle>& triangles) {
    if (triangles.empty()) {
        return 0.0;
    }

    const AABB bounds = computeMeshBounds(triangles);
    const double boxVolume = boundsVolume(bounds);
    if (boxVolume <= kEpsilon) {
        return 0.0;
    }

    std::size_t nx = 0;
    std::size_t ny = 0;
    std::size_t nz = 0;
    computeGridResolution(bounds, nx, ny, nz);

    const Vector3 ext = boundsExtent(bounds);
    const double dx = ext.x / static_cast<double>(nx);
    const double dy = ext.y / static_cast<double>(ny);
    const double dz = ext.z / static_cast<double>(nz);

    const std::vector<OctreeNode> octree = buildOctree(triangles, bounds);
    const Vector3 rayDir = normalize(Vector3{1.0, 0.371, 0.529}, kEpsilon);

    std::vector<std::uint32_t> visitedStamp(triangles.size(), 0);
    std::uint32_t stamp = 1;

    std::size_t insideCount = 0;
    std::size_t totalCount = 0;

    for (std::size_t ix = 0; ix < nx; ++ix) {
        const double x = bounds.min.x + (static_cast<double>(ix) + 0.5) * dx;
        for (std::size_t iy = 0; iy < ny; ++iy) {
            const double y = bounds.min.y + (static_cast<double>(iy) + 0.5) * dy;
            for (std::size_t iz = 0; iz < nz; ++iz) {
                const double z = bounds.min.z + (static_cast<double>(iz) + 0.5) * dz;
                const Vector3 point{x, y, z};

                if (pointInsideMesh(point, triangles, octree, rayDir, visitedStamp, stamp)) {
                    ++insideCount;
                }
                ++totalCount;
                ++stamp;
                if (stamp == 0) {
                    std::fill(visitedStamp.begin(), visitedStamp.end(), 0);
                    stamp = 1;
                }
            }
        }
    }

    if (totalCount == 0) {
        return 0.0;
    }

    const double fractionInside = static_cast<double>(insideCount) / static_cast<double>(totalCount);
    return fractionInside * boxVolume;
}
