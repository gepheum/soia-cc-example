#!/bin/bash

set -e

find . -not -path 'skirout/*' -iname '*.cc' -o -iname '*.h' | xargs clang-format -i
npx skir gen
bazel build :all
bazel test :all
