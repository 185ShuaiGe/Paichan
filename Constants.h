#pragma once

// --- 常量定义 ---

// 遗传算法参数
const int POPULATION_SIZE = 5;     // 种群大小
const int MAX_GENERATIONS = 50;    // 最大迭代代数
const double MUTATION_RATE = 0.1;  // 变异率
const double CROSSOVER_RATE = 0.8; // 交叉率
const int TOURNAMENT_SIZE = 2;     // 锦标赛选择大小

// 生产约束
const double MAX_WEIGHT_PER_BATCH = 2200.0; // 每道次最大总重
const double MIN_WEIGHT_PER_BATCH = 1700.0; // 每道次最小总重
const int MAX_BATCHES_PER_DAY = 15;         // 每日最大道次
const int MAX_ZD_BRICKS_FIRST_5_DAYS = 20;  // 前五天ZD砖块每日最大生产数

// 惩罚项权重
const double PENALTY_WEIGHT_VIOLATION = 1.0;       // 重量约束违反惩罚
const double PENALTY_UNFINISHED_PRODUCTION = 10.0; // 未完成生产的惩罚
const double PENALTY_ZD_FIRST_5_DAYS = 2.0;        // 前五天ZD生产规则违反惩罚
const double PENALTY_TYPE_DIVERSITY = 1.0;         // 砖型多样性惩罚
const double PENALTY_ODD_PRODUCTION = 1.0;         // 奇数生产惩罚
