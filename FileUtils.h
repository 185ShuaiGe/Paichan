#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <ctime>
#include <filesystem> // C++17 标准库，用于创建目录

#include "DataStructures.h"
#include "Constants.h"

// read_brick_data 函数保持不变
std::vector<Brick> read_brick_data(const std::string &filename)
{
    std::vector<Brick> bricks;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return bricks;
    }
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string cell;
        Brick b;
        try
        {
            std::getline(ss, cell, ',');
            b.id = std::stoi(cell);
            std::getline(ss, b.type, ',');
            std::getline(ss, cell, ',');
            b.quantity = std::stoi(cell);
            std::getline(ss, cell, ',');
            b.weight = std::stod(cell);
            std::getline(ss, cell, ',');
            b.priority = std::stoi(cell);
            bricks.push_back(b);
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Warning: Skipping invalid line in CSV: " << line << " (" << e.what() << ")" << std::endl;
        }
    }
    return bricks;
}

// 重写CSV写入函数，正确处理周日并输出到指定文件夹
void write_schedule_to_csv(const Individual &best_solution, const std::vector<Brick> &bricks, const std::string &filename)
{
    // 确保输出目录存在
    std::filesystem::create_directory(OUTPUT_DIR);
    std::ofstream file(OUTPUT_DIR + filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for writing: " << OUTPUT_DIR + filename << std::endl;
        return;
    }

    // --- 写入表头 (包含周日) ---
    file << "序号,砖型,总需求,单重(kg),优先级,,,,,,";

    std::tm start_tm = {};
    start_tm.tm_year = 2024 - 1900;
    start_tm.tm_mon = 7 - 1;
    start_tm.tm_mday = 8;
    start_tm.tm_isdst = -1;
    std::time_t current_time_t = std::mktime(&start_tm);

    int work_days_covered = 0;
    while (work_days_covered < best_solution.actual_days)
    {
        std::tm *current_tm = std::localtime(&current_time_t);

        char date_buffer[11];
        std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", current_tm);
        file << date_buffer << ",";

        // 如果不是周日 (tm_wday != 0), 则计为一个工作日
        if (current_tm->tm_wday != 0)
        {
            work_days_covered++;
        }

        current_time_t += 86400; // 移动到日历上的下一天
    }
    file << "\n";

    // --- 写入数据行 ---
    for (size_t i = 0; i < bricks.size(); ++i)
    {
        file << bricks[i].id << "," << bricks[i].type << "," << bricks[i].quantity << "," << bricks[i].weight << "," << bricks[i].priority << ",,,,,,,";

        // 重新初始化日期，与表头对齐
        current_time_t = std::mktime(&start_tm);
        work_days_covered = 0;
        int current_work_day_idx = 0;

        while (work_days_covered < best_solution.actual_days)
        {
            std::tm *current_tm = std::localtime(&current_time_t);

            if (current_tm->tm_wday != 0)
            { // 如果是工作日
                if (current_work_day_idx < best_solution.actual_days)
                {
                    int daily_total_quantity = 0;
                    for (int pass = 0; pass < PASSES_PER_DAY; ++pass)
                    {
                        daily_total_quantity += best_solution.schedule[current_work_day_idx][pass][i];
                    }
                    if (daily_total_quantity > 0)
                    {
                        file << daily_total_quantity;
                    }
                }
                current_work_day_idx++;
                work_days_covered++;
            }
            // 无论是工作日还是休息日，都要输出一个逗号来占位
            file << ",";

            current_time_t += 86400;
        }
        file << "\n";
    }

    file.close();
}

#endif // FILE_UTILS_H
