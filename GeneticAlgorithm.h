#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <map>
#include <cmath>
#include <numeric>

#include "Constants.h"
#include "DataStructures.h"

// 声明一个外部的随机数生成器，它将在main.cpp中被定义
extern std::mt19937 rng;

// --- 遗传算法函数 ---

void calculate_fitness(Individual &individual, const std::vector<Brick> &bricks)
{
    double penalty = 0.0;
    std::vector<double> produced_count(bricks.size(), 0.0);

    int days = individual.schedule.size();
    individual.actual_days = days; // 存储真实天数

    std::vector<double> produced_before_today(bricks.size(), 0.0);

    for (int d = 0; d < days; ++d)
    {
        double daily_zd_count = 0;
        int types_produced_today = 0;

        double total_daily_weight = 0;
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            double amount = individual.schedule[d][i];
            if (amount > 0)
            {
                total_daily_weight += amount * bricks[i].weight;
                types_produced_today++;
            }
        }
        int estimated_batches = std::ceil(total_daily_weight / ((MAX_WEIGHT_PER_BATCH + MIN_WEIGHT_PER_BATCH) / 2.0));
        if (estimated_batches > MAX_BATCHES_PER_DAY)
        {
            penalty += PENALTY_WEIGHT_VIOLATION * (estimated_batches - MAX_BATCHES_PER_DAY);
        }

        int unfinished_types_before_today = 0;
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            if (produced_before_today[i] < bricks[i].total_count)
            {
                unfinished_types_before_today++;
            }
        }

        for (size_t i = 0; i < bricks.size(); ++i)
        {
            double amount = individual.schedule[d][i];
            if (amount <= 0)
                continue;

            if (d < 5)
            {
                if (bricks[i].priority != 0)
                    penalty += PENALTY_ZD_FIRST_5_DAYS;
                daily_zd_count += amount;
            }

            if (d >= 5 && unfinished_types_before_today > 10 && types_produced_today < 10)
            {
                penalty += PENALTY_TYPE_DIVERSITY * (10 - types_produced_today);
            }

            double remaining_before = bricks[i].total_count - produced_before_today[i];
            if (remaining_before > 1 && static_cast<int>(std::round(amount)) % 2 != 0 && amount > 0.1)
            {
                penalty += PENALTY_ODD_PRODUCTION;
            }

            produced_count[i] += amount;
        }

        if (d < 5 && daily_zd_count > MAX_ZD_BRICKS_FIRST_5_DAYS)
        {
            penalty += PENALTY_ZD_FIRST_5_DAYS * (daily_zd_count - MAX_ZD_BRICKS_FIRST_5_DAYS);
        }

        for (size_t i = 0; i < bricks.size(); ++i)
        {
            produced_before_today[i] += individual.schedule[d][i];
        }
    }

    for (size_t i = 0; i < bricks.size(); ++i)
    {
        if (produced_count[i] < bricks[i].total_count)
        {
            penalty += PENALTY_UNFINISHED_PRODUCTION * (bricks[i].total_count - produced_count[i]);
        }
    }

    individual.fitness = static_cast<double>(days) + penalty;
}

Individual generate_greedy_solution(const std::vector<Brick> &bricks, int max_days = 100)
{
    Individual individual;
    std::vector<double> produced_count(bricks.size(), 0.0);
    int day = 0;

    std::vector<int> brick_indices(bricks.size());
    std::iota(brick_indices.begin(), brick_indices.end(), 0);
    std::sort(brick_indices.begin(), brick_indices.end(), [&](int a, int b)
              { return bricks[a].priority < bricks[b].priority; });

    while (day < max_days)
    {
        bool all_done = true;
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            if (produced_count[i] < bricks[i].total_count)
            {
                all_done = false;
                break;
            }
        }
        if (all_done)
            break;

        individual.schedule.emplace_back(std::vector<double>(bricks.size(), 0.0));
        double daily_total_weight = 0;
        int estimated_batches = 0;

        if (day < 5)
        {
            double zd_produced_today = 0;
            for (int idx : brick_indices)
            {
                if (bricks[idx].priority != 0 || produced_count[idx] >= bricks[idx].total_count)
                    continue;
                double remaining_to_produce = bricks[idx].total_count - produced_count[idx];
                double can_produce = std::min(remaining_to_produce, static_cast<double>(MAX_ZD_BRICKS_FIRST_5_DAYS - zd_produced_today));
                if (can_produce <= 0)
                    continue;
                if (remaining_to_produce > 1 && can_produce > 1)
                    can_produce = std::floor(can_produce / 2.0) * 2.0;
                if (can_produce <= 0)
                    continue;
                individual.schedule[day][idx] = can_produce;
                produced_count[idx] += can_produce;
                zd_produced_today += can_produce;
            }
        }
        else
        {
            for (int idx : brick_indices)
            {
                if (produced_count[idx] >= bricks[idx].total_count || estimated_batches >= MAX_BATCHES_PER_DAY)
                    continue;
                double remaining_weight_capacity = MAX_BATCHES_PER_DAY * MAX_WEIGHT_PER_BATCH - daily_total_weight;
                if (remaining_weight_capacity <= 0)
                    break;
                double remaining_to_produce = bricks[idx].total_count - produced_count[idx];
                double can_produce_by_weight = remaining_weight_capacity / bricks[idx].weight;
                double amount_to_produce = std::floor(std::min(remaining_to_produce, can_produce_by_weight));
                if (amount_to_produce < 1.0)
                    continue;
                if (remaining_to_produce > 1 && amount_to_produce > 1)
                    amount_to_produce = std::floor(amount_to_produce / 2.0) * 2.0;
                if (amount_to_produce <= 0)
                    continue;
                individual.schedule[day][idx] = amount_to_produce;
                produced_count[idx] += amount_to_produce;
                daily_total_weight += amount_to_produce * bricks[idx].weight;
                estimated_batches = std::ceil(daily_total_weight / ((MAX_WEIGHT_PER_BATCH + MIN_WEIGHT_PER_BATCH) / 2.0));
            }
        }
        day++;
    }
    if (day >= max_days)
        std::cout << "Warning: Greedy solution exceeds max days." << std::endl;
    return individual;
}

void mutate(Individual &individual, const std::vector<Brick> &bricks)
{
    if (individual.schedule.empty())
        return;
    std::uniform_int_distribution<> day_dist(0, individual.schedule.size() - 1);
    std::uniform_int_distribution<> brick_dist(0, bricks.size() - 1);
    int day_idx = day_dist(rng);
    int brick_idx = brick_dist(rng);
    double produced_before_day = 0.0;
    for (int d = 0; d < day_idx; ++d)
        produced_before_day += individual.schedule[d][brick_idx];
    double remaining_total = bricks[brick_idx].total_count - produced_before_day;
    if (remaining_total <= 0)
        return;
    std::uniform_real_distribution<> amount_dist(0, remaining_total);
    double new_amount = std::floor(amount_dist(rng));
    if (remaining_total > 1 && new_amount > 1)
        new_amount = std::floor(new_amount / 2.0) * 2.0;
    individual.schedule[day_idx][brick_idx] = new_amount;

    std::vector<double> total_produced(bricks.size(), 0.0);
    for (auto &day_plan : individual.schedule)
    {
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            double can_produce = bricks[i].total_count - total_produced[i];
            day_plan[i] = std::max(0.0, std::min(day_plan[i], can_produce));
            total_produced[i] += day_plan[i];
        }
    }
}

std::vector<Individual> initialize_population(const std::vector<Brick> &bricks)
{
    std::vector<Individual> population;
    std::cout << "Generating initial greedy solution..." << std::endl;
    Individual greedy_sol = generate_greedy_solution(bricks);
    population.push_back(greedy_sol);
    std::cout << "Creating initial population from mutations..." << std::endl;
    for (int i = 1; i < POPULATION_SIZE; ++i)
    {
        Individual new_ind = greedy_sol;
        for (int j = 0; j < 10; ++j)
            mutate(new_ind, bricks);
        population.push_back(new_ind);
    }
    return population;
}

Individual tournament_selection(const std::vector<Individual> &population)
{
    Individual best;
    std::uniform_int_distribution<> dist(0, population.size() - 1);
    int best_idx = dist(rng);
    best = population[best_idx];
    for (int i = 1; i < TOURNAMENT_SIZE; ++i)
    {
        int idx = dist(rng);
        if (population[idx].fitness < best.fitness)
            best = population[idx];
    }
    return best;
}

Individual crossover(const Individual &parent1, const Individual &parent2, const std::vector<Brick> &bricks)
{
    Individual offspring;
    const auto &p1 = (parent1.schedule.size() < parent2.schedule.size()) ? parent1 : parent2;
    const auto &p2 = (parent1.schedule.size() < parent2.schedule.size()) ? parent2 : parent1;
    size_t p1_days = p1.schedule.size();
    if (p1_days <= 1)
        return p2;

    std::uniform_int_distribution<> dist(0, p1_days - 1);
    int crossover_point = dist(rng);

    offspring.schedule.insert(offspring.schedule.end(), p1.schedule.begin(), p1.schedule.begin() + crossover_point);
    offspring.schedule.insert(offspring.schedule.end(), p2.schedule.begin() + crossover_point, p2.schedule.end());

    std::vector<double> total_produced(bricks.size(), 0.0);
    for (auto &day_plan : offspring.schedule)
    {
        for (size_t i = 0; i < bricks.size(); ++i)
        {
            double can_produce = bricks[i].total_count - total_produced[i];
            day_plan[i] = std::max(0.0, std::min(day_plan[i], can_produce));
            total_produced[i] += day_plan[i];
        }
    }
    return offspring;
}
