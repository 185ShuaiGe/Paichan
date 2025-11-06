#pragma once

#include <vector>
#include <string>

// --- 数据结构 ---

struct Brick
{
    int id;
    std::string type;
    double total_count;
    double weight;
    int priority; // 0 for ZD, 1 for ZB, 2 for ZF
};

// 代表一个个体（一种排产方案）
struct Individual
{
    // schedule[day][brick_id] = produced_amount
    std::vector<std::vector<double>> schedule;
    double fitness = -1.0;
    int actual_days = 0; // 存储真实的、不含惩罚的天数
};
