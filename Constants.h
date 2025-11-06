#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// --- 文件路径 ---
const std::string OUTPUT_DIR = "results/";

// --- 遗传算法参数 ---
const int POPULATION_SIZE = 50;  // 增加种群大小以应对更严的约束
const int MAX_GENERATIONS = 200; // 增加迭代次数
const double MUTATION_RATE = 0.2;
const double CROSSOVER_RATE = 0.8;
const int TOURNAMENT_SIZE = 5;

// --- 生产约束 ---
const int MAX_DAYS = 50;
const int PASSES_PER_DAY = 15;
const double MIN_PASS_WEIGHT = 1700.0;
const double MAX_PASS_WEIGHT = 2200.0;
const int MAX_BRICKS_PER_PASS = 4; // 新增：每个道次最多生产的砖块总数

// --- 分阶段生产参数 ---
const int PHASE1_DAYS = 5;
const int PHASE2_MIN_DIVERSITY = 10;

// --- 适应度函数惩罚权重 ---
const double PASS_OVERWEIGHT_PENALTY_WEIGHT = 20.0;
const double PASS_UNDERWEIGHT_PENALTY_WEIGHT = 15.0;
const double PASS_QUANTITY_PENALTY_WEIGHT = 50.0; // 新增：道次数量超限惩罚 (高权重)
const double PRIORITY_PENALTY_WEIGHT = 30.0;
const double DIVERSITY_PENALTY_WEIGHT = 5.0;
const double UNMET_DEMAND_PENALTY_WEIGHT = 1000.0;

#endif // CONSTANTS_H
