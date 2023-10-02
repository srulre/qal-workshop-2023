#include "arithmetic_oracle.hpp"
#include "arithmetic_register.hpp"
#include "classiq.hpp"
#include "declarations.hpp"
#include "make_qfunc.hpp"
#include "synthesize.hpp"
#include "visualization.hpp"

#include <iostream>

namespace classiq = cudaq::classiq;

int main()
{
    auto my_state_preparation = classiq::make_qfunc("my_state_preparation",
                                                    [](classiq::qreg<"target"> qreg)
                                                    { classiq::apply_to_all(qreg.size(), classiq::h, qreg); });

    auto my_oracle =
          classiq::arithmetic_oracle({classiq::arithmetic_register{"a", 2}, classiq::arithmetic_register{"b", 2}},
                                     "(a + b == 5) and (a - b == 1)");
    auto my_oracle_op = [&](classiq::qreg<"oracle"> q)
    {
        my_oracle(q);
    };

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
                              });
          });

    auto main = classiq::make_qfunc("main",
                                    [&]()
                                    {
                                        auto all_qubits = classiq::allocate<"grover">(my_oracle.size());
                                        grover_search(1, all_qubits, my_state_preparation, my_oracle_op);
                                    });

    auto model = classiq::model(main);
    auto [kernel, qubit_mapping] = classiq::synthesize(model);
    auto counts = cudaq::sample(kernel);
    counts.dump();
    classiq::visualize(kernel);
}