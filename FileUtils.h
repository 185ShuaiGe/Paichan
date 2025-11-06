#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "DataStructures.h" // 依赖于自定义的数据结构

// --- 函数声明与实现 ---

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
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string item;
        Brick b;

        std::getline(ss, item, ',');
        b.id = std::stoi(item);
        std::getline(ss, item, ',');
        b.type = item;
        std::getline(ss, item, ',');
        b.total_count = std::stod(item);
        std::getline(ss, item, ',');
        b.weight = std::stod(item);

        if (b.type.rfind("ZD", 0) == 0)
            b.priority = 0;
        else if (b.type.rfind("ZB", 0) == 0)
            b.priority = 1;
        else
            b.priority = 2;

        bricks.push_back(b);
    }
    file.close();
    return bricks;
}

void write_schedule_to_csv(const Individual &individual, const std::vector<Brick> &bricks, const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not create output file " << filename << std::endl;
        return;
    }

    // 写入表头
    file << "序号,砖型,砖数,重量,,,,,,"; // 7个空列

    int month = 7;
    int day_of_month = 8;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int working_day_counter = 0;

    int total_days_to_print = std::ceil(static_cast<double>(individual.actual_days) * 7.0 / 6.0);

    for (int i = 0; i < total_days_to_print; ++i)
    {
        file << month << "." << day_of_month << ",";
        day_of_month++;
        if (day_of_month > days_in_month[month])
        {
            day_of_month = 1;
            month++;
        }
    }
    file << "\n";

    // 写入数据
    for (const auto &b : bricks)
    {
        file << b.id << "," << b.type << "," << b.total_count << "," << b.weight << ",,,,,,,,"; // 7 empty columns

        working_day_counter = 0;
        day_of_month = 8;
        month = 7;

        for (int i = 2; i < total_days_to_print; ++i)
        {
            // 每工作6天休息一天 (i从0开始，所以i=6是第7天)
            if (i % 7 == 0)
            { // Rest day logic
              // This corresponds to day 7, 14, 21...
            }
            else
            {
                if (working_day_counter < individual.schedule.size())
                {
                    double amount = individual.schedule[working_day_counter][b.id - 1];
                    if (amount > 0.001)
                    { // Use a small epsilon for floating point comparison
                        file << amount;
                    }
                }
                working_day_counter++;
            }
            file << ",";

            day_of_month++;
            if (day_of_month > days_in_month[month])
            {
                day_of_month = 1;
                month++;
            }
        }
        file << "\n";
    }
    file.close();
}
