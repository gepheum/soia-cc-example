# Skir C++ example

Example showing how to use skir's [C++ code generator](https://github.com/gepheum/skir-cc-gen) in a project.

## Build and run the example

### Install bazel and NodeJS

If you use homebrew:

```shell
brew install bazel
brew install node
```

This example uses Bazel as a layer on top of your compiler for building and running the C++ code as well as a C++ dependency manager.

### Download this repository

```shell
git clone https://github.com/gepheum/skir-cc-example.git
cd skir-cc-example
```

### Run Skir-to-C++ code generation

`npx skir gen`

The Skir compiler looks for a `skir.yml` file in the current directory.
This file contains the following information:

*    `srcDir`: path to the directory containing all the .skir files
*    `generators`: code generators to run; each generator targets one programming language

The compiler creates a `skirout` directory containing all the generated code.

Alternatively, you can run the compiler with watch mode on: `npx skir gen -w`.
While the process is running, every modification to a .skir file in the source
directory will trigger code generation. 
The process won't stop until you terminate it.

### Build and run the C++ code

```shell
# Runs example.cc
# Contains code snippets showing how to use the skir-generated data types
bazel run :example

# Unit tests for skir-generated data types
bazel test :example.test

# Starts a skir service
bazel run :service_start

# Sends RPCs to the skir service started with ^
# Run this command from a different process
bazel run :service_client
```

## IDE support

### VSCode

If your C++ compiler is clang (recommended), install the `clangd` extension and disable Microsoft's C/C++ extension.

Every time you change the rules in `BUILD.bazel`, run `bazel run @hedron_compile_commands//:refresh_all`.
This will regenerate the `compile_commands.json` file, which will give you:

*   code completion
*   compile errors and warnings
*   go-to-definition and cross references
*   hover information and inlay hints
*   include management
*   code formatting
*   simple refactorings
