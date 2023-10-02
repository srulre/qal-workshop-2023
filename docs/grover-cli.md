# Second Example - Grover with CLI

We extended the previous example, so instead of hardcoded 
parameters, it will run according to user input.

Here, the focus is to observe the outcome resulting from modifying 
different parameters, such as the oracle definition and the 
constraints.

## CLI Syntax

The CLI syntax is given as follows:

1. `oracle` - required: an arithmetic oracle expression and the 
   variable declarations.
   - For the list of supported arithmetic operators, see Classiq 
     documentation on [arithmetic expressions](https://docs.classiq.io/latest/user-guide/function-library/builtin-functions/arithmetic/arithmetic-expression/).
   - Example: `--oracle "a + b == 11 and (a - b == 1 or b - a == 1)" a 3 b 3`.
   - Note: currently, the variables are unsigned integers only.
2. `iterations` - the number of Grover iterations. Default: `1`.
3. `optimization-parameter` - a quantitative parameter provided 
   to the synthesis engine to optimize the circuit by. Can get the 
   following arguments: 
   - `width` - Total number of qubits. Prefers implementations 
     with a minimal number of qubits and reusing released qubits 
     instead of allocating new ones.
   - `depth` - Circuit depth. Prefers implementations with a higher 
     number of qubits and will avoid reusing qubits.
   - `cx` - CX gate count. Prefers implementations with a higher 
     number of qubits and does not care about qubit reuse.
   **It is recommended to optimize by width when performing 
     sampling**.
   - Example: `--optimization-parameter width`
4. `max-width`, `max-depth`, `max-cx-count` - numerical 
   limitations on the quantities.
   - Example: `--max-width 14`
5. `sample-circuit` - a flag that gets `true` (default) or `false`.
    **It is recommended to avoid sampling if not optimizing by 
   width**.
6. `visualize-circuit` - a flag that gets `true` (default) or `false`.

## Running the Code

In the Visual Studio Code terminal, inside the `qal_workshop` 
folder, run `./build/grover_cli --oracle ORACLE [optional args]`.

## Suggested Exercises

1. Try sampling different oracles - vary the arithmetic 
   expressions, variable sizes, and the number of variables.
2. Try spotting the diffuser and isolating the block that belongs 
   to the multi-controlled Z operation (currently, it's not marked 
   explicitly, but it will be in the future).
3. Pick an oracle of size at least 8. Here, size refers to the 
   total sum of the registers of the variables. Optimize the CX 
   gate count and visualize the circuit. What is the resulting 
   number of qubits? How many qubits does the multi-controlled Z gate have?
4. Repeat the process, but now optimize the width.
5. Try limiting the gate-count to a ridiculously small number (e.g.
   , 1). What is the resulting error message?