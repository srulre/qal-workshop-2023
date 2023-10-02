#include "grover_cli_tools.hpp"

#include <cstdio>
#include <iostream>

#include <boost/program_options.hpp>

classiq::AllConstraints deduce_constraint(const std::string& arg)
{
    if (arg == "width")
    {
        return classiq::constraints::Basic::WIDTH;
    }
    else if (arg == "depth")
    {
        return classiq::constraints::Basic::DEPTH;
    }
    else if (arg == "cx")
    {
        return classiq::constraints::GateCount::CX;
    }
    else
    {
        throw std::runtime_error("Invalid constraint token");
    }
}

std::tuple<std::vector<std::string>, size_t, classiq::Constraints, bool, bool> add_cli_options(int argc, char* argv[])
{
    auto constraints = classiq::Constraints();
    size_t iterations;
    bool to_visualize;
    bool to_sample;
    std::vector<std::string> oracle;

    po::options_description desc("Allowed options");
    po::variables_map vm;

    desc.add_options()("help", "Produce help message")("version", "Show program version")(
          "oracle, or", po::value<std::vector<std::string>>(&oracle)->required()->multitoken(), "Arithmetic oracle")(
          "iterations, n", po::value<size_t>(&iterations)->default_value(1), "Grover iterations")(
          "max-width, w",
          po::value<size_t>()->notifier([&constraints](size_t max_width)
                                        { constraints.set(classiq::constraints::Basic::WIDTH, max_width); }),
          "Maximum width")(
          "max-depth, d",
          po::value<size_t>()->notifier([&constraints](size_t max_depth)
                                        { constraints.set(classiq::constraints::Basic::DEPTH, max_depth); }),
          "Maximum depth")(
          "max-cx-count, cx",
          po::value<size_t>()->notifier([&constraints](size_t max_cx_count)
                                        { constraints.set(classiq::constraints::GateCount::CX, max_cx_count); }),
          "Maximum CX gate count")(
          "optimization-parameter, opt",
          po::value<std::string>()->notifier([&](std::string constraint)
                                             { constraints.optimize_by(deduce_constraint(constraint)); }),
          "Optimization parameter")(
          "sample-circuit, smp", po::value<bool>(&to_sample)->default_value(true), "Sample resulting circuit")(
          "visualize-circuit, vis", po::value<bool>(&to_visualize)->default_value(true), "Visualize resulting circuit");

    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        exit(1);
    }
    if (vm.count("version"))
    {
        std::cout << "0.0.0" << std::endl;
        exit(1);
    }

    po::notify(vm);

    return {oracle, iterations, constraints, to_sample, to_visualize};
}

std::pair<std::string, std::unordered_map<std::string, size_t>> parse_oracle(const std::vector<std::string>& cli_oracle)
{

    if (cli_oracle.size() % 2 == 0)
    {
        throw std::invalid_argument("Usage: '[expression] [name_0] [size_0] [name_1] [size_1]...'");
    }
    std::string expr(cli_oracle.at(0));
    std::unordered_map<std::string, size_t> name_to_size_map;

    std::string arg_name;
    std::string arg_size;
    for (size_t i = 1; i < cli_oracle.size(); i += 2)
    {
        auto name_id = i;
        arg_name = cli_oracle[name_id];
        arg_size = cli_oracle[name_id + 1];
        try
        {
            size_t pos;
            name_to_size_map[arg_name] = (size_t)std::stoi(arg_size, &pos);
            if (pos != arg_size.length())
            {
                throw std::invalid_argument("Did not parse entire string");
            }
        }
        catch (const std::invalid_argument& err)
        {
            throw std::invalid_argument("Usage: '[expression] [name_0] [size_0] "
                                        "[name_1] [size_1]...'\n" +
                                        arg_size + " is not a valid integer");
        }
    }
    return {expr, name_to_size_map};
}