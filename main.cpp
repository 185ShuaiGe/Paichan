#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>    // 新增：用于文件流操作
#include <filesystem> // 新增：用于创建目录

// 包含所有自定义模块
#include "Constants.h"
#include "DataStructures.h"
#include "FileUtils.h"
#include "GeneticAlgorithm.h"

// 定义在GeneticAlgorithm.h中声明的全局随机数生成器
std::mt19937 rng(std::random_device{}());

int main()
{
    // --- 准备输出目录 ---
    const std::string output_dir = "results/";
    try
    {
        std::filesystem::create_directory(output_dir);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
    }

    // 1. 读取数据
    auto bricks = read_brick_data("data_input_converted.csv");
    if (bricks.empty())
    {
        return 1;
    }
    std::cout << "Read " << bricks.size() << " types of bricks." << std::endl;

    // --- 准备迭代过程日志文件 ---
    std::ofstream progress_log(output_dir + "progress_log.csv");
    if (!progress_log.is_open())
    {
        std::cerr << "Error: Could not create progress_log.csv" << std::endl;
    }
    else
    {
        // 写入CSV表头
        progress_log << "Generation,BestFitness,ActualDays\n";
    }

    // 2. 初始化种群
    std::cout << "Initializing population..." << std::endl;
    auto population = initialize_population(bricks);

    Individual best_overall_solution = population[0];
    calculate_fitness(best_overall_solution, bricks);

    // 3. 遗传算法主循环
    for (int gen = 0; gen < MAX_GENERATIONS; ++gen)
    {
        for (auto &individual : population)
        {
            calculate_fitness(individual, bricks);
        }

        std::sort(population.begin(), population.end(), [](const Individual &a, const Individual &b)
                  { return a.fitness < b.fitness; });

        if (population[0].fitness < best_overall_solution.fitness)
        {
            best_overall_solution = population[0];
        }

        std::cout << "Generation " << gen << ": Best Fitness = " << best_overall_solution.fitness
                  << " (Production Days: " << best_overall_solution.actual_days << ")" << std::endl;

        // --- 将本代数据写入日志文件 ---
        if (progress_log.is_open())
        {
            progress_log << gen << "," << best_overall_solution.fitness << "," << best_overall_solution.actual_days << "\n";
        }

        // 生成下一代种群
        std::vector<Individual> new_population;
        new_population.push_back(best_overall_solution);

        while (new_population.size() < POPULATION_SIZE)
        {
            Individual parent1 = tournament_selection(population);
            Individual parent2 = tournament_selection(population);

            Individual offspring = parent1;
            std::uniform_real_distribution<> cross_dist(0.0, 1.0);
            if (cross_dist(rng) < CROSSOVER_RATE)
            {
                offspring = crossover(parent1, parent2, bricks);
            }

            std::uniform_real_distribution<> mut_dist(0.0, 1.0);
            if (mut_dist(rng) < MUTATION_RATE)
            {
                mutate(offspring, bricks);
            }
            new_population.push_back(offspring);
        }
        population = new_population;
    }

    // 关闭日志文件
    if (progress_log.is_open())
    {
        progress_log.close();
    }

    // 4. 输出最终结果
    calculate_fitness(best_overall_solution, bricks);
    std::cout << "\n--- Optimization Finished ---\n";

    int total_period_with_rest = std::ceil(static_cast<double>(best_overall_solution.actual_days) * 7.0 / 6.0);
    std::cout << "  - Total Production Period (including rest days): " << total_period_with_rest << " days.\n";
    std::cout << "  - Total Working Days: " << best_overall_solution.actual_days << " days.\n";
    std::cout << "  - Final Fitness Score: " << best_overall_solution.fitness << "\n";

    // 将最终计划写入results目录
    write_schedule_to_csv(best_overall_solution, bricks, output_dir + "production_plan.csv");
    std::cout << "Optimal production plan saved to " << output_dir << "production_plan.csv" << std::endl;
    std::cout << "Optimization progress log saved to " << output_dir << "progress_log.csv" << std::endl;

    return 0;
}
