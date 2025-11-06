#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <map>
#include <filesystem> // 用于创建目录
#include <fstream>    // 用于文件流

#include "DataStructures.h"
#include "Constants.h"
#include "FileUtils.h"
#include "GeneticAlgorithm.h"

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

int main()
{
    // 确保输出目录存在
    try
    {
        std::filesystem::create_directory(OUTPUT_DIR);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return 1;
    }

    // 1. 读取数据
    std::vector<Brick> bricks = read_brick_data("bricks_data.csv");
    if (bricks.empty())
        return 1;
    size_t num_bricks = bricks.size();

    // 2. 第一阶段
    std::cout << "Phase 1: Pre-scheduling first " << PHASE1_DAYS << " days..." << std::endl;
    Individual base_schedule(num_bricks);
    std::vector<int> remaining_quantities(num_bricks);
    for (size_t i = 0; i < num_bricks; ++i)
        remaining_quantities[i] = bricks[i].quantity;
    pre_schedule_phase1(base_schedule, bricks, remaining_quantities);

    // 3. 准备第二阶段
    std::map<int, int> phase2_demands;
    for (size_t i = 0; i < num_bricks; ++i)
    {
        if (remaining_quantities[i] > 0)
            phase2_demands[i] = remaining_quantities[i];
    }
    std::cout << "Phase 1 complete. " << phase2_demands.size() << " brick types have remaining demand for Phase 2." << std::endl;

    // 4. 第二阶段：遗传算法
    std::cout << "Phase 2: Running Genetic Algorithm..." << std::endl;
    std::ofstream log_file(OUTPUT_DIR + "progress_log.csv");
    log_file << "Generation,BestFitness,TotalDays\n";

    std::vector<Individual> population = initialize_population_phase2(bricks, phase2_demands, base_schedule);

    for (int gen = 0; gen < MAX_GENERATIONS; ++gen)
    {
        for (auto &ind : population)
        {
            calculate_fitness_phase2(ind, bricks, phase2_demands);
        }

        std::sort(population.begin(), population.end(), [](const Individual &a, const Individual &b)
                  { return a.fitness < b.fitness; });

        std::vector<Individual> new_population;
        new_population.push_back(population[0]);

        while (new_population.size() < POPULATION_SIZE)
        {
            Individual parent1 = tournament_selection(population);
            Individual parent2 = tournament_selection(population);
            Individual offspring = parent1;
            if ((double)rand() / RAND_MAX < CROSSOVER_RATE)
            {
                offspring = crossover_phase2(parent1, parent2, bricks);
            }
            if ((double)rand() / RAND_MAX < MUTATION_RATE)
            {
                mutate_phase2(offspring, bricks, phase2_demands);
            }
            new_population.push_back(offspring);
        }
        population = new_population;

        if ((gen + 1) % 100 == 0)
        {
            std::cout << "Generation " << gen + 1 << "/" << MAX_GENERATIONS << ", Best Fitness: " << population[0].fitness << ", Total Days: " << population[0].actual_days << std::endl;
            log_file << gen + 1 << "," << population[0].fitness << "," << population[0].actual_days << "\n";
        }
    }
    log_file.close();

    // 5. 最终结果
    Individual best_solution = population[0];
    calculate_fitness_phase2(best_solution, bricks, phase2_demands);

    std::cout << "\nOptimization finished." << std::endl;
    std::cout << "Best solution found with total duration: " << best_solution.actual_days << " working days." << std::endl;

    write_schedule_to_csv(best_solution, bricks, "production_plan.csv");
    std::cout << "Production plan and progress log saved to '" << OUTPUT_DIR << "' folder." << std::endl;

    return 0;
}
