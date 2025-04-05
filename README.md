# Soia C++ examples

Examples showing how to use soia [C++ code generation](https://github.com/gepheum/soia-cc-gen) in a C++ project.

## Build and run the examples

### Install NodeJS and bazel

If you use homebrew:

```
brew install node
brew install bazel
```

The soia compiler runs on NodeJS.

This example uses Bazel as a layer on top of your compiler for building and running the C++ code as well as a C++ dependency manager.

### Download this repository

```
git clone https://github.com/gepheum/soia-cc-examples.git
cd soia-cc-examples
```

### Install the soia compiler in your project

`npm i`

This command will fetch and install the two dependencies specified in `package.json`:

*   `soiac`: the soia compiler
*   `soia-cc-gen`: the soia plugin for generating C++ code

You only need to run this command once.

### Run the soia compiler

`npm run soiac`

The compiler looks for a `soia.yml` file in the current directory.
This file contains the following information:

*    `soiaSrc`: path to the root of the directory containing all the .soia files
*    `generators`: code generators to run; each generator targets one programming language

The compiler creates a `soiagen` directory containing all the generated code.

Alternatively, you can run the compiler with watch mode on: `npm run soiac -- -w`.
While the process is running, every modification to a .soia file in the source
directory will trigger code generation. 
The process won't stop until you terminate it.

### Build and run the C++ code

```
# Runs example.cc
# Contains code snippets showing how to use the soia-generated data types
bazel run :example

# Unit tests for soia-generated data types
bazel test :example.text

# Start an HTTP server with a soia API
bazel run :soia_server

# Call the soia API started with ^
# Run this command from a different process
bazel run :soia_client
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
