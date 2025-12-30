#!/bin/bash

set -e

find e2e-test -not -path 'skirout/*' -iname '*.cc' -o -iname '*.h' | xargs clang-format -i
npx skir gen
bazel build :all
bazel test :all
