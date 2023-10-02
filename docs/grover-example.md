# First Example - Grover

For a Grover example in CUDA Quantum, see 
[CUDA Quantum documentation](https://nvidia.github.io/cuda-quantum/latest/specification/cudaq/examples.html#grover-s-algorithm).

The first part focuses on constructing a simple example of 
Grover's algorithm with an arithmetic oracle. Here, the goal is 
to present Classiq's built-in library and show CUDA kernel 
visualization. 

The next session will discuss constraints in detail.

In this example, we provide a hardcoded circuit as follows:

1. The oracle is the following arithmetic expression `a + b == 5 
   and a - b == 1`, where `a` and `b` are unsigned integers. 
   This, of course, evaluates to `a = 3` and `b = 2`.
2. We perform a single iteration.
3. The state preparation is a uniform superposition (default).

Before we run the example, let's break down its different parts. 

## The Oracle

A computational basis oracle performs the transformation

\[
\left|\psi\right> = \sum_{x=\left\{0,1\right\}^{n}}\psi_{x}
\left|x\right> \rightarrow
\sum_{x=\left\{0,1\right\}^{n}}\psi_{x}\left(-1\right)^{f\left(x\right)}
\left|x\right>
\]

In the canonical Grover algorithm, the coefficients $\psi_{x}$ are 
uniform, so


\[ 
\left|\psi\right> = \frac{1}{\sqrt{2^{n}}}\sum_
{x=\left\{0,1\right\}^{n}}\left|x\right>,
\] 


Here, we use an arithmetic oracle, so $f\left(x\right) = 1$ for 
$x$ that satisfies some arithmetic condition. 

Note that the arithmetic oracle is a Classiq built-in function that 
automatically generates the corresponding unitary operator. The 
function gets variable declarations and an arithmetic expression 
as arguments.

```cpp
   auto my_oracle = classiq::arithmetic_oracle(
      {
         classiq::arithmetic_register{"a", 2}, 
         classiq::arithmetic_register{"b", 2}
      },
      "(a + b == 5) and (a - b == 1)"
   );
```

## The Diffuser

The Grover's diffuser "reflects" around a chosen quantum state 
$\left|\psi\right>$, by applying the operator
$2\left|\psi\right>\left<\psi\right| - \mathbf{I}$.

### The State Preparation

The state preparation is a unitary operator that performs the 
transformation $\left|0\right> \rightarrow \left|\psi\right>$.

Here, we show the canonical example, where the uniform 
superposition state is obtained by applying Hadamard gates on all 
qubits.

In this code, let's focus on the last line: it shows the internal
Classiq's `apply_to_all` construct.

```cpp
    auto my_state_preparation = classiq::make_qfunc(
      "my_state_preparation",
      [](classiq::qreg<"target"> qreg)
      { classiq::apply_to_all(qreg.size(), classiq::h, qreg); }
    );
```

## The Full Diffuser

The diffuser gets a state preparation as an argument, so it can 
reflect about states other than the uniform superposition. Here,
we meet two additional constructs: `control` and `invert`.

The main focus here is on the last three statements.

```cpp
    auto compute_block = classiq::make_qfunc(
        "compute_block",
        [](
            classiq::qreg<"block"> qreg,
            classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation
        )
        {
            classiq::apply_to_all(qreg.size(), classiq::x, qreg);
            state_preparation(qreg);
        }
    );

    auto my_diffuser = classiq::make_qfunc(
          "my_diffuser",
          [&compute_block](
              classiq::qreg<"diff"> diffuser_qreg,
              classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation
          )
          {
              classiq::invert(
                  diffuser_qreg, 
                  [&](classiq::qreg<"target"> q) { 
                     compute_block(q, state_preparation); 
                  }
              );
              classiq::control(
                  diffuser_qreg.back(), 
                  diffuser_qreg.front(diffuser_qreg.size() - 1), 
                  classiq::z
              );
              compute_block(diffuser_qreg, state_preparation);
          }
    );
```

## The Grover Search - Putting Everything Together

Here, we meet the construct `repeat` that concatenates the Grover 
operators according to the number of repetitions provided by the 
user. 

Note: 

1. `apply_to_all` is a special case of `repeat`.
2. `my_diffuser` is an external function captured by the grover 
   search (by using `[&my_diffuser]`). The state preparation 
   operand is propagated to the diffuser.

```cpp
   auto grover_search = classiq::make_qfunc(
      "my_grover_search",
      [&my_diffuser](
         classiq::param<"reps", size_t> reps,
         classiq::qreg<"grover"> qreg,
         classiq::operand<"state_preparation", classiq::qreg<"target">> state_preparation,
         classiq::operand<"oracle", classiq::qreg<"oracle">> oracle
         )
         {
            state_preparation(qreg);
            classiq::repeat(
               qreg,
               reps,
               [&](classiq::param<"index", size_t>, classiq::qreg<"qbv"> q)
               {
               oracle(q);
               my_diffuser(q, state_preparation);
               }
            );
         }
      );
```

## The Entry Point

The entry point calls the Grover search. 
By using `[&]`, we capture the previously defined functions 
`my_state_preparation` and `my_oracle_op` into the call. Here, we 
perform a single Grover iteration.

```cpp
    auto main = classiq::make_qfunc(
      "main",
      [&]()
      {
         auto all_qubits = classiq::allocate<"grover">(my_oracle.size());
         grover_search(1, all_qubits, my_state_preparation, my_oracle_op);
      }
    );
```

## Executing the Flow

Now, we construct the model, call the Classiq synthesis 
engine, run the synthesized circuit, and visualize it:

```cpp
auto model = classiq::model(main);
auto [kernel, qubit_mapping] = classiq::synthesize(model);

auto counts = cudaq::sample(kernel);
counts.dump();

classiq::visualize(kernel);
```

## Running the Code

In the Visual Studio Code terminal, inside the `qal_workshop` 
folder, run `./build/grover_example`.