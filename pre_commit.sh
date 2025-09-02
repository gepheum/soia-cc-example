#!/bin/bash

set -e

find e2e-test -not -path 'soiagen/*' -iname '*.cc' -o -iname '*.h' | xargs clang-format -i
npm i
npm run soiac
bazel build :all
bazel test :all
