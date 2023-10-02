#include "arithmetic_oracle.hpp"
#include "classiq.hpp"
#include "declarations.hpp"
#include "grover_cli_tools.hpp"
#include "make_qfunc.hpp"
#include "model.hpp"
#include "synthesize.hpp"
#include "visualization.hpp"

#include <cstdio>
#include <iostream>
#include <cudaq.h>

#include <boost/program_options.hpp>

namespace classiq = cudaq::classiq;

int main(int argc, char* argv[])
{
    auto [cli_oracle, iterations, constraints, to_sample, to_visualize] = add_cli_options(argc, argv);
    auto [oracle_expression, oracle_register_to_size] = parse_oracle(cli_oracle);

    auto my_state_preparation = classiq::make_qfunc("my_state_preparation",
                                                    [](classiq::qreg<"target"> qreg)
                                                    { classiq::apply_to_all(qreg.size(), classiq::h, qreg); });

    auto compute_block =
          classiq::make_qfunc("compute_block",
                              [](classiq::qreg<"block"> qreg,
                                 classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation)
                              {
                                  classiq::apply_to_all(qreg.size(), classiq::x, qreg);
                                  state_preparation(qreg);
                              });

    auto my_diffuser = classiq::make_qfunc(
          "my_diffuser",
          [&compute_block](classiq::qreg<"diff"> diffuser_qreg,
                           classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation)
          {
              classiq::invert(diffuser_qreg, [&](classiq::qreg<"target"> q) { compute_block(q, state_preparation); });
              classiq::control(diffuser_qreg.back(), diffuser_qreg.front(diffuser_qreg.size() - 1), classiq::z);
              compute_block(diffuser_qreg, state_preparation);
          });

    auto grover_search = classiq::make_qfunc(
          "my_grover_search",
          [&my_diffuser](classiq::param<"reps", size_t> reps,
                         classiq::qreg<"grover"> qreg,
                         classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation,
                         classiq::operand<"oracle", classiq::qreg<"oracle">> oracle)
          {
              state_preparation(qreg);
              classiq::repeat(qreg,
                              reps,
                              [&](classiq::param<"index", size_t>, classiq::qreg<"qbv"> q)
                              {
                                  oracle(q);
                                  my_diffuser(q, state_preparation);
                                  classiq::u(q.front(), 0, 0, 0, classiq::constants::pi);
                              });
          });

    std::vector<classiq::arithmetic_register> oracle_registers;
    oracle_registers.reserve(oracle_register_to_size.size());
    std::ranges::transform(oracle_register_to_size,
                           std::back_inserter(oracle_registers),
                           [](const std::pair<std::string, std::size_t>& reg)
                           {
                               const auto [reg_name, reg_size] = reg;
                               return classiq::arithmetic_register(reg_name, reg_size);
                           });

    auto my_oracle = classiq::arithmetic_oracle(oracle_registers, oracle_expression);
    auto my_oracle_op = [&](classiq::qreg<"oracle"> q)
    {
        my_oracle(q);
    };

    auto main = classiq::make_qfunc("main",
                                    [&]()
                                    {
                                        auto all_qubits = classiq::allocate<"grover">(my_oracle.size());
                                        grover_search(iterations, all_qubits, my_state_preparation, my_oracle_op);
                                    });

    auto model = classiq::model(main, constraints);
    auto [kernel, qubit_mapping] = classiq::synthesize(model);
    if (to_sample)
    {
        auto counts = cudaq::sample(kernel);
        counts.dump();
    }
    if (to_visualize)
    {
        classiq::visualize(kernel);
    }
}