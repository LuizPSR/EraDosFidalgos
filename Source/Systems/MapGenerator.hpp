#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>
#include <vector>
#include <array>
#include <cmath>
#include <random>
#include <queue>
#include <algorithm>

#include "Components/Province.hpp"
#include "Components/Culture.hpp"
#include <SDL3/SDL.h>

// Map constants
constexpr int MAP_WIDTH = 90;
constexpr int MAP_HEIGHT = 45;

// Terrain thresholds (normalized 0-1)
constexpr float WATER_THRESHOLD = 0.30f;
constexpr float WETLANDS_THRESHOLD = 0.34f;
constexpr float PLAINS_THRESHOLD = 0.57f;
constexpr float HILLS_THRESHOLD = 0.80f;

// Biome assignment constants
constexpr float HOT_THRESHOLD = 0.35f;
constexpr float DRY_THRESHOLD_HOT = 0.35f;
constexpr float DRY_THRESHOLD_COLD = 0.45f;
constexpr float ELEVATION_TEMP_REDUCTION = 0.3f;
constexpr float TEMPERATURE_NOISE_SCALE = 0.3f;
constexpr float DRYNESS_NOISE_SCALE = 0.3f;

// Culture expansion constants
constexpr float BASE_TRAVEL_COST = 5.0f;
constexpr float TERRAIN_BIOME_MISMATCH_PENALTY = 50.0f;

// Noise generation constants
constexpr int PERLIN_TABLE_SIZE = 512;
constexpr float NOISE_SCALE_BASE = 16.0f;
constexpr float NOISE_SCALE_MULTIPLIER = 2.0f;

// Components
struct TileMap {
    std::vector<std::vector<flecs::entity>> tiles;
    int width = MAP_WIDTH;
    int height = MAP_HEIGHT;
};

struct TileData {
    int x;
    int y;
    float height_value;
};

struct CultureData {
    CultureType culture;
};

// Perlin noise implementation
namespace {

class PerlinNoise {
private:
    std::array<int, PERLIN_TABLE_SIZE> permutation;

    static float fade(float t) {
        return 6.0f * t * t * t * t * t - 15.0f * t * t * t * t + 10.0f * t * t * t;
    }

    static float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    static float gradient(int hash, float x, float y) {
        const std::array<std::array<float, 2>, 4> vectors = {{
            {0.0f, 1.0f},
            {0.0f, -1.0f},
            {1.0f, 0.0f},
            {-1.0f, 0.0f}
        }};
        const auto& g = vectors[hash % 4];
        return g[0] * x + g[1] * y;
    }

public:
    explicit PerlinNoise(uint32_t seed) {
        std::mt19937 rng(seed);
        for (int i = 0; i < 256; ++i) {
            permutation[i] = i;
        }
        std::shuffle(permutation.begin(), permutation.begin() + 256, rng);
        for (int i = 0; i < 256; ++i) {
            permutation[256 + i] = permutation[i];
        }
    }

    float noise(float x, float y) const {
        int xi = static_cast<int>(std::floor(x)) & 255;
        int yi = static_cast<int>(std::floor(y)) & 255;
        float xf = x - std::floor(x);
        float yf = y - std::floor(y);

        float g00 = gradient(permutation[xi + permutation[yi]], xf, yf);
        float g01 = gradient(permutation[xi + permutation[yi + 1]], xf, yf - 1.0f);
        float g10 = gradient(permutation[xi + 1 + permutation[yi]], xf - 1.0f, yf);
        float g11 = gradient(permutation[xi + 1 + permutation[yi + 1]], xf - 1.0f, yf - 1.0f);

        float t = fade(xf);
        float u = fade(yf);
        return lerp(u, lerp(t, g00, g10), lerp(t, g01, g11));
    }
};

std::vector<float> get_noise_scales() {
    std::vector<float> scales;
    float scale = std::min(MAP_WIDTH, MAP_HEIGHT) / NOISE_SCALE_BASE;
    while (scale < MAP_WIDTH && scale < MAP_HEIGHT) {
        scales.push_back(scale);
        scale *= NOISE_SCALE_MULTIPLIER;
    }
    return scales;
}

void generate_height_map(uint32_t seed, std::vector<std::vector<float>>& height_map) {
    PerlinNoise noise(seed);
    auto scales = get_noise_scales();

    // Generate noise
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            float value = 0.0f;
            for (float scale : scales) {
                value += noise.noise(x / scale, y / scale);
            }
            height_map[x][y] = value;
        }
    }

    // Find min/max for normalization
    float min_val = height_map[0][0];
    float max_val = height_map[0][0];
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            min_val = std::min(min_val, height_map[x][y]);
            max_val = std::max(max_val, height_map[x][y]);
        }
    }

    // Apply edge falloff and normalize
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            height_map[x][y] -= min_val;
            float interpolation = std::sin(x / float(MAP_WIDTH) * M_PI) + 
                                std::sin(y / float(MAP_HEIGHT) * M_PI);
            height_map[x][y] *= interpolation * interpolation;
        }
    }

    // Final normalization
    min_val = height_map[0][0];
    max_val = height_map[0][0];
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            min_val = std::min(min_val, height_map[x][y]);
            max_val = std::max(max_val, height_map[x][y]);
        }
    }
    
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            height_map[x][y] = (height_map[x][y] - min_val) / (max_val - min_val);
        }
    }
}

TerrainType height_to_terrain(float height) {
    if (height <= WATER_THRESHOLD) return Sea;
    if (height <= WETLANDS_THRESHOLD) return Wetlands;
    if (height <= PLAINS_THRESHOLD) return Plains;
    if (height <= HILLS_THRESHOLD) return Hills;
    return Mountains;
}

void label_terrain(const std::vector<std::vector<float>>& height_map,
                  std::vector<std::vector<TerrainType>>& terrain_map) {
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            terrain_map[x][y] = height_to_terrain(height_map[x][y]);
        }
    }
}

int flood_fill(const std::vector<std::vector<TerrainType>>& terrain_map,
               std::vector<std::vector<int>>& labels, int start_x, int start_y, int label) {
    std::queue<std::pair<int, int>> q;
    q.push({start_x, start_y});
    labels[start_x][start_y] = label;
    int size = 0;

    const std::array<std::pair<int, int>, 4> directions = {{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();
        size++;

        for (auto [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT &&
                terrain_map[nx][ny] != Sea && labels[nx][ny] == 0) {
                labels[nx][ny] = label;
                q.push({nx, ny});
            }
        }
    }

    return size;
}

void keep_largest_landmass(std::vector<std::vector<TerrainType>>& terrain_map) {
    std::vector<std::vector<int>> labels(MAP_WIDTH, std::vector<int>(MAP_HEIGHT, 0));
    int current_label = 1;
    std::vector<int> label_sizes;

    // Flood fill to label connected landmasses
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (terrain_map[x][y] != Sea && labels[x][y] == 0) {
                int size = flood_fill(terrain_map, labels, x, y, current_label);
                label_sizes.push_back(size);
                current_label++;
            }
        }
    }

    if (label_sizes.empty()) return;

    // Find largest landmass
    int largest_label = 1;
    int largest_size = label_sizes[0];
    for (size_t i = 1; i < label_sizes.size(); ++i) {
        if (label_sizes[i] > largest_size) {
            largest_size = label_sizes[i];
            largest_label = i + 1;
        }
    }

    // Convert everything else to water
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (labels[x][y] != largest_label && labels[x][y] != 0) {
                terrain_map[x][y] = Sea;
            }
        }
    }
}

void calculate_distance_from_water(const std::vector<std::vector<TerrainType>>& terrain_map,
                                  std::vector<std::vector<float>>& distance_map) {
    // Initialize distances
    std::queue<std::pair<int, int>> q;
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (terrain_map[x][y] == Sea) {
                distance_map[x][y] = 0.0f;
                q.push({x, y});
            } else {
                distance_map[x][y] = std::numeric_limits<float>::infinity();
            }
        }
    }

    // BFS to compute distances
    const std::array<std::pair<int, int>, 4> directions = {{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (auto [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                float new_dist = distance_map[x][y] + 1.0f;
                if (new_dist < distance_map[nx][ny]) {
                    distance_map[nx][ny] = new_dist;
                    q.push({nx, ny});
                }
            }
        }
    }

    // Normalize distances
    float max_dist = 0.0f;
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (distance_map[x][y] != std::numeric_limits<float>::infinity()) {
                max_dist = std::max(max_dist, distance_map[x][y]);
            }
        }
    }

    if (max_dist > 0.0f) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            for (int y = 0; y < MAP_HEIGHT; ++y) {
                if (distance_map[x][y] != std::numeric_limits<float>::infinity()) {
                    distance_map[x][y] /= max_dist;
                }
            }
        }
    }
}

void assign_biomes(uint32_t seed,
                  const std::vector<std::vector<TerrainType>>& terrain_map,
                  const std::vector<std::vector<float>>& height_map,
                  std::vector<std::vector<BiomeType>>& biome_map) {
    std::vector<std::vector<float>> distance_map(MAP_WIDTH, std::vector<float>(MAP_HEIGHT));
    calculate_distance_from_water(terrain_map, distance_map);

    PerlinNoise noise(seed + 1000);

    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (terrain_map[x][y] == Sea) {
                biome_map[x][y] = Water;
                continue;
            }

            float latitude_factor = float(x) / MAP_WIDTH;
            float distance_factor = distance_map[x][y];
            float elevation_factor = height_map[x][y];

            float noise_val = noise.noise(x / 20.0f, y / 20.0f);
            float noise_normalized = (noise_val + 1.0f) / 2.0f;

            float temperature = latitude_factor - (elevation_factor * ELEVATION_TEMP_REDUCTION) + 
                              (noise_normalized - 0.5f) * TEMPERATURE_NOISE_SCALE;

            float coldness_factor = 1.0f - temperature;
            float noise_val2 = noise.noise(x / 15.0f, y / 15.0f);
            float noise_normalized2 = (noise_val2 + 1.0f) / 2.0f;
            float dryness = (distance_factor * 0.5f) + (elevation_factor * 0.3f) + 
                          (coldness_factor * 0.2f) + (noise_normalized2 - 0.5f) * DRYNESS_NOISE_SCALE;

            if (temperature > HOT_THRESHOLD) {
                biome_map[x][y] = (dryness > DRY_THRESHOLD_HOT) ? Drylands : Jungles;
            } else {
                biome_map[x][y] = (dryness > DRY_THRESHOLD_COLD) ? Grasslands : Forests;
            }
        }
    }
}

struct BiomeClassification {
    bool dry;
    bool hot;
};

BiomeClassification classify_biome(BiomeType biome) {
    bool dry = (biome == Drylands || biome == Grasslands);
    bool hot = (biome == Drylands || biome == Jungles);
    return {dry, hot};
}

int get_terrain_biome_match(TerrainType terrain, BiomeType biome,
                           TerrainType preferred_terrain, BiomeType preferred_biome) {
    int terrain_distance = std::abs(static_cast<int>(preferred_terrain) - static_cast<int>(terrain));
    
    auto [dry, hot] = classify_biome(biome);
    auto [pref_dry, pref_hot] = classify_biome(preferred_biome);
    
    int biome_distance;
    if (dry == pref_dry && hot == pref_hot) {
        biome_distance = 2;
    } else if (dry == pref_dry || hot == pref_hot) {
        biome_distance = 1;
    } else {
        biome_distance = 0;
    }
    
    return terrain_distance + biome_distance;
}

float calculate_travel_cost(TerrainType terrain, BiomeType biome, CultureType culture) {
    if (terrain == Sea) {
        return std::numeric_limits<float>::infinity();
    }

    auto traits = GetCulturalTraits(culture);
    int mismatch = get_terrain_biome_match(terrain, biome, traits.preferred_terrain, traits.preferred_biome);
    
    return BASE_TRAVEL_COST + (TERRAIN_BIOME_MISMATCH_PENALTY * mismatch);
}

void assign_cultures(uint32_t seed,
                    const std::vector<std::vector<TerrainType>>& terrain_map,
                    std::vector<std::vector<BiomeType>>& biome_map,
                    std::vector<std::vector<CultureType>>& culture_map) {
    std::mt19937 rng(seed + 2000);

    // Culture preferences
    struct CulturePreference {
        CultureType culture;
        TerrainType terrain;
        BiomeType biome;
    };

    std::vector<CulturePreference> cultures = {
        {SteppeNomads, Plains, Grasslands},
        {ForestFolk, Plains, Forests},
        {HillDwellers, Hills, Grasslands},
        {FarmLanders, Plains, Jungles}
    };

    std::shuffle(cultures.begin(), cultures.end(), rng);

    // Define corners
    struct Corner {
        int x_min, x_max, y_min, y_max;
    };
    std::vector<Corner> corners = {
        {0, MAP_WIDTH/2, 0, MAP_HEIGHT/2},
        {MAP_WIDTH/2, MAP_WIDTH, 0, MAP_HEIGHT/2},
        {0, MAP_WIDTH/2, MAP_HEIGHT/2, MAP_HEIGHT},
        {MAP_WIDTH/2, MAP_WIDTH, MAP_HEIGHT/2, MAP_HEIGHT}
    };
    std::shuffle(corners.begin(), corners.end(), rng);

    // Find culture centers
    std::vector<std::tuple<int, int, CultureType>> centers;
    for (size_t i = 0; i < cultures.size(); ++i) {
        const auto& pref = cultures[i];
        const auto& corner = corners[i % corners.size()];

        std::vector<std::pair<int, int>> matching_tiles;
        for (int x = corner.x_min; x < corner.x_max; ++x) {
            for (int y = corner.y_min; y < corner.y_max; ++y) {
                if (terrain_map[x][y] == pref.terrain && biome_map[x][y] == pref.biome) {
                    matching_tiles.push_back({x, y});
                }
            }
        }

        // Fallback to any land
        if (matching_tiles.empty()) {
            for (int x = corner.x_min; x < corner.x_max; ++x) {
                for (int y = corner.y_min; y < corner.y_max; ++y) {
                    if (terrain_map[x][y] != Sea) {
                        matching_tiles.push_back({x, y});
                    }
                }
            }
        }

        if (!matching_tiles.empty()) {
            std::uniform_int_distribution<size_t> dist(0, matching_tiles.size() - 1);
            auto [cx, cy] = matching_tiles[dist(rng)];
            centers.push_back({cx, cy, pref.culture});
        }
    }

    // Multi-source Dijkstra
    std::vector<std::vector<float>> cost_map(MAP_WIDTH, std::vector<float>(MAP_HEIGHT, 
                                             std::numeric_limits<float>::infinity()));
    
    using QueueElement = std::tuple<float, int, int, CultureType>;
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<>> pq;

    for (auto [cx, cy, culture] : centers) {
        cost_map[cx][cy] = 0.0f;
        culture_map[cx][cy] = culture;
        pq.push({0.0f, cx, cy, culture});
    }

    const std::array<std::pair<int, int>, 4> directions = {{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}};

    while (!pq.empty()) {
        auto [current_cost, x, y, current_culture] = pq.top();
        pq.pop();

        if (current_cost > cost_map[x][y]) {
            continue;
        }

        for (auto [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                float travel_cost = calculate_travel_cost(terrain_map[nx][ny], 
                                                         biome_map[nx][ny], 
                                                         current_culture);
                float new_cost = current_cost + travel_cost;

                if (new_cost < cost_map[nx][ny]) {
                    cost_map[nx][ny] = new_cost;
                    culture_map[nx][ny] = current_culture;
                    pq.push({new_cost, nx, ny, current_culture});
                }
            }
        }
    }

    // Post-processing: FarmLanders conversion
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            if (culture_map[x][y] == FarmLanders) {
                if (biome_map[x][y] == Forests || biome_map[x][y] == Jungles) {
                    if (dist(rng) < 0.5f) {
                        biome_map[x][y] = Grasslands;
                    }
                }
            }
        }
    }
}

} // anonymous namespace

static void GenerateMap(const flecs::world &ecs, uint32_t seed) {
    // Create height map
    std::vector<std::vector<float>> height_map(MAP_WIDTH, std::vector<float>(MAP_HEIGHT));
    generate_height_map(seed, height_map);

    // Create terrain map
    std::vector<std::vector<TerrainType>> terrain_map(MAP_WIDTH, std::vector<TerrainType>(MAP_HEIGHT));
    label_terrain(height_map, terrain_map);
    keep_largest_landmass(terrain_map);

    // Create biome map
    std::vector<std::vector<BiomeType>> biome_map(MAP_WIDTH, std::vector<BiomeType>(MAP_HEIGHT));
    assign_biomes(seed, terrain_map, height_map, biome_map);

    // Create culture map
    std::vector<std::vector<CultureType>> culture_map(MAP_WIDTH,
                                                      std::vector<CultureType>(MAP_HEIGHT, SteppeNomads));
    assign_cultures(seed, terrain_map, biome_map, culture_map);

    // Create tilemap entity
    auto tilemap_entity = ecs.entity("TileMap");
    auto& tilemap = tilemap_entity.ensure<TileMap>();
    tilemap.tiles.resize(MAP_WIDTH, std::vector<flecs::entity>(MAP_HEIGHT));

    // Create province entities for each tile
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            auto tile = ecs.entity().child_of(tilemap_entity);

            auto& province = tile.ensure<Province>();
            province.mPosX = x;
            province.mPosY = y;
            province.name = "Province_" + std::to_string(x) + "_" + std::to_string(y);
            province.terrain = terrain_map[x][y];
            province.biome = biome_map[x][y];
            province.development = (terrain_map[x][y] != Sea) ? 10 : 0;
            province.control = (terrain_map[x][y] != Sea) ? 100 : 0;

            province.culture = culture_map[x][y];

            province.movement_cost = 30
                +  15 * (province.terrain == Plains)
                +  15 * (province.terrain == Mountains)
                +  15 * (province.roads_level == 0 && (province.biome == Forests || province.biome == Jungles))
                -  5 * province.roads_level;

            auto& tile_data = tile.ensure<TileData>();
            tile_data.x = x;
            tile_data.y = y;
            tile_data.height_value = height_map[x][y];

            if (terrain_map[x][y] != Sea) {
                auto& culture_data = tile.ensure<CultureData>();
                culture_data.culture = culture_map[x][y];
            }

            tilemap.tiles[x][y] = tile;
        }
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Map Generated Successfully");
}
