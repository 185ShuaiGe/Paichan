#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <map>
#include <set>
#include "DataStructures.h"
#include "Constants.h"

extern std::mt19937 rng;

// ==============================================================================
// 1. 第一阶段预处理函数 (加入道次数量检查)
// ==============================================================================
void pre_schedule_phase1(Individual &individual, const std::vector<Brick> &bricks, std::vector<int> &remaining_quantities)
{
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);

    for (int day = 0; day < PHASE1_DAYS; ++day)
    {
        // 记录每个道次的当前负载 (重量和数量)
        std::vector<double> pass_weights(PASSES_PER_DAY, 0.0);
        std::vector<int> pass_quantities(PASSES_PER_DAY, 0);

        for (size_t i = 0; i < bricks.size(); ++i)
        {
            if (bricks[i].type.rfind("ZD", 0) == 0 && remaining_quantities[i] > 0)
            {
                if (prob_dist(rng) < 0.90)
                {
                    int quantity_to_produce = 2;
                    if (remaining_quantities[i] >= quantity_to_produce)
                    {
                        double batch_weight = quantity_to_produce * bricks[i].weight;

                        for (int pass = 0; pass < PASSES_PER_DAY; ++pass)
                        {
                            if (pass_weights[pass] + batch_weight <= MAX_PASS_WEIGHT &&
                                pass_quantities[pass] + quantity_to_produce <= MAX_BRICKS_PER_PASS)
                            {
                                individual.schedule[day][pass][i] += quantity_to_produce;
                                pass_weights[pass] += batch_weight;
                                pass_quantities[pass] += quantity_to_produce;
                                remaining_quantities[i] -= quantity_to_produce;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ==============================================================================
// 2. 第二阶段遗传算法函数 (全部加入道次数量检查)
// ==============================================================================

// --- 适应度函数 ---
void calculate_fitness_phase2(Individual &individual, const std::vector<Brick> &bricks, const std::map<int, int> &phase2_demands)
{
    double total_penalty = 0;
    individual.actual_days = PHASE1_DAYS;
    std::map<int, int> total_produced_phase2;

    int remaining_types_count = phase2_demands.size();
    int min_diversity_target = std::min(PHASE2_MIN_DIVERSITY, remaining_types_count);

    for (int day = PHASE1_DAYS; day < MAX_DAYS; ++day)
    {
        double daily_production_weight = 0;
        std::set<int> types_produced_this_day;

        for (int pass = 0; pass < PASSES_PER_DAY; ++pass)
        {
            double pass_weight = 0;
            int pass_quantity = 0; // 新增：计算道次总数量
            for (size_t i = 0; i < bricks.size(); ++i)
            {
                int quantity = individual.schedule[day][pass][i];
                if (quantity > 0)
                {
                    pass_weight += quantity * bricks[i].weight;
                    pass_quantity += quantity; // 累加数量
                    types_produced_this_day.insert(i);
                    total_produced_phase2[i] += quantity;
                }
            }
            // 惩罚1: 道次约束 (重量 + 数量)
            if (pass_weight > 0)
            {
                if (pass_weight > MAX_PASS_WEIGHT)
                    total_penalty += (pass_weight - MAX_PASS_WEIGHT) * PASS_OVERWEIGHT_PENALTY_WEIGHT;
                else if (pass_weight < MIN_PASS_WEIGHT)
                    total_penalty += (MIN_PASS_WEIGHT - pass_weight) * PASS_UNDERWEIGHT_PENALTY_WEIGHT;
            }
            if (pass_quantity > MAX_BRICKS_PER_PASS)
            {
                total_penalty += (pass_quantity - MAX_BRICKS_PER_PASS) * PASS_QUANTITY_PENALTY_WEIGHT;
            }
            daily_production_weight += pass_weight;
        }

        if (daily_production_weight > 0)
        {
            individual.actual_days = day + 1;
            if (types_produced_this_day.size() < min_diversity_target)
            {
                total_penalty += (min_diversity_target - types_produced_this_day.size()) * DIVERSITY_PENALTY_WEIGHT;
            }
        }
    }

    // ... (优先级和未满足需求惩罚逻辑不变) ...
    int first_day_zb_zf = MAX_DAYS;
    int last_day_zd = 0;
    for (int day = PHASE1_DAYS; day < individual.actual_days; ++day)
    {
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            bool produced_this_day = false;
            for (int pass = 0; pass < PASSES_PER_DAY; ++pass)
                if (individual.schedule[day][pass][i] > 0)
                    produced_this_day = true;

            if (produced_this_day)
            {
                if (bricks[i].type.rfind("ZD", 0) == 0 && phase2_demands.count(i))
                    last_day_zd = std::max(last_day_zd, day);
                if (bricks[i].type.rfind("ZB", 0) == 0 || bricks[i].type.rfind("ZF", 0) == 0)
                    first_day_zb_zf = std::min(first_day_zb_zf, day);
            }
        }
    }
    if (first_day_zb_zf < last_day_zd)
    {
        total_penalty += (last_day_zd - first_day_zb_zf) * PRIORITY_PENALTY_WEIGHT;
    }
    for (const auto &pair : phase2_demands)
    {
        int brick_idx = pair.first;
        int demand = pair.second;
        total_penalty += std::abs(total_produced_phase2[brick_idx] - demand) * UNMET_DEMAND_PENALTY_WEIGHT;
    }

    individual.fitness = (individual.actual_days - PHASE1_DAYS) + total_penalty;
}

// --- 种群初始化 ---
std::vector<Individual> initialize_population_phase2(const std::vector<Brick> &bricks, const std::map<int, int> &phase2_demands, const Individual &base_schedule)
{
    std::vector<Individual> population;
    std::uniform_int_distribution<> batch_size_dist(1, 2); // 1->2件, 2->4件

    for (int i = 0; i < POPULATION_SIZE; ++i)
    {
        Individual ind = base_schedule;

        std::vector<std::pair<int, int>> batches_to_schedule;
        for (const auto &pair : phase2_demands)
        {
            int brick_idx = pair.first;
            int remaining_qty = pair.second;
            while (remaining_qty > 0)
            {
                int batch_size = batch_size_dist(rng) * 2;
                batch_size = std::min(remaining_qty, batch_size);
                batches_to_schedule.push_back({brick_idx, batch_size});
                remaining_qty -= batch_size;
            }
        }
        std::shuffle(batches_to_schedule.begin(), batches_to_schedule.end(), rng);

        for (const auto &batch : batches_to_schedule)
        {
            int brick_idx = batch.first;
            int qty = batch.second;
            double batch_weight = qty * bricks[brick_idx].weight;

            std::uniform_int_distribution<> day_dist(PHASE1_DAYS, MAX_DAYS - 1);
            std::uniform_int_distribution<> pass_dist(0, PASSES_PER_DAY - 1);

            for (int attempt = 0; attempt < 100; ++attempt)
            {
                int day = day_dist(rng);
                int pass = pass_dist(rng);

                double current_pass_weight = 0;
                int current_pass_quantity = 0;
                for (size_t j = 0; j < bricks.size(); ++j)
                {
                    current_pass_weight += ind.schedule[day][pass][j] * bricks[j].weight;
                    current_pass_quantity += ind.schedule[day][pass][j];
                }

                if (current_pass_weight + batch_weight <= MAX_PASS_WEIGHT &&
                    current_pass_quantity + qty <= MAX_BRICKS_PER_PASS)
                {
                    ind.schedule[day][pass][brick_idx] += qty;
                    break;
                }
            }
        }
        population.push_back(ind);
    }
    return population;
}

// --- 变异 ---
void mutate_phase2(Individual &individual, const std::vector<Brick> &bricks, const std::map<int, int> &phase2_demands)
{
    if (phase2_demands.empty())
        return;

    std::uniform_int_distribution<> day_dist(PHASE1_DAYS, MAX_DAYS - 1);
    std::uniform_int_distribution<> pass_dist(0, PASSES_PER_DAY - 1);
    std::vector<int> demand_indices;
    for (const auto &pair : phase2_demands)
        demand_indices.push_back(pair.first);
    std::uniform_int_distribution<> brick_dist(0, demand_indices.size() - 1);

    int from_day = day_dist(rng);
    int from_pass = pass_dist(rng);
    int brick_idx = demand_indices[brick_dist(rng)];

    if (individual.schedule[from_day][from_pass][brick_idx] > 0)
    {
        int amount_to_move = (std::uniform_int_distribution<>(1, 2)(rng)) * 2;
        amount_to_move = std::min(individual.schedule[from_day][from_pass][brick_idx], amount_to_move);

        int to_day = day_dist(rng);
        int to_pass = pass_dist(rng);

        if (from_day == to_day && from_pass == to_pass)
            return;

        // 检查目标道次容量
        double to_pass_weight = 0;
        int to_pass_quantity = 0;
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            to_pass_weight += individual.schedule[to_day][to_pass][i] * bricks[i].weight;
            to_pass_quantity += individual.schedule[to_day][to_pass][i];
        }

        if (to_pass_weight + amount_to_move * bricks[brick_idx].weight <= MAX_PASS_WEIGHT &&
            to_pass_quantity + amount_to_move <= MAX_BRICKS_PER_PASS)
        {
            individual.schedule[from_day][from_pass][brick_idx] -= amount_to_move;
            individual.schedule[to_day][to_pass][brick_idx] += amount_to_move;
        }
    }
}

// Crossover 和 Tournament Selection 函数保持不变
// ...

Individual crossover_phase2(const Individual &parent1, const Individual &parent2, const std::vector<Brick> &bricks)
{
    Individual offspring(bricks.size());
    offspring = parent1;

    std::uniform_int_distribution<> dist(0, bricks.size() - 1);
    int crossover_point = dist(rng);

    for (size_t i = crossover_point; i < bricks.size(); ++i)
    {
        for (int day = PHASE1_DAYS; day < MAX_DAYS; ++day)
        {
            for (int pass = 0; pass < PASSES_PER_DAY; ++pass)
            {
                offspring.schedule[day][pass][i] = parent2.schedule[day][pass][i];
            }
        }
    }
    return offspring;
}

Individual tournament_selection(const std::vector<Individual> &population)
{
    Individual best(population.empty() ? 0 : population[0].schedule[0][0].size());
    bool first = true;
    std::uniform_int_distribution<> dist(0, population.size() - 1);

    for (int i = 0; i < TOURNAMENT_SIZE; ++i)
    {
        const Individual &contender = population[dist(rng)];
        if (first || contender.fitness < best.fitness)
        {
            best = contender;
            first = false;
        }
    }
    return best;
}

#endif // GENETIC_ALGORITHM_H
