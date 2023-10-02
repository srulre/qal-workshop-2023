compile_exe(
    NAME grover_example
    SOURCES src/grover_example.cpp
    LINK_LIBRARIES classiq synthesize visualization)

compile_exe(
    NAME grover_cli
    SOURCES src/grover_cli.cpp src/grover_cli_tools.cpp
    LINK_LIBRARIES classiq synthesize visualization Boost::program_options)