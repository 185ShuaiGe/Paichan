#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>
#include <string>
#include "Constants.h"

struct Brick
{
    int id;
    std::string type;
    int quantity;
    double weight;
    int priority;
};

struct Individual
{
    // 核心改动：schedule变为三维数组 [天][道次][砖型]
    std::vector<std::vector<std::vector<int>>> schedule;
    double fitness;
    int actual_days;

    Individual() : fitness(0.0), actual_days(0) {}

    // 构造函数需要知道砖块总数以正确初始化
    Individual(int num_bricks) : fitness(0.0), actual_days(0)
    {
        schedule.resize(MAX_DAYS,
                        std::vector<std::vector<int>>(PASSES_PER_DAY,
                                                      std::vector<int>(num_bricks, 0)));
    }
};

#endif // DATA_STRUCTURES_H
