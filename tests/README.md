# AshDB Tests

## Code Coverage

Code coverage is currently only supported on Mac. To generate a coverage report locally run the following commands from the build directory.

From the buil directory run:
```
cmake --build . --verbose -- -j 18
ctest --verbose
lcov -d .. -c -o coverage.info
lcov -r coverage.info "*Xcode.app*" "*/test/*" "*Qt*" "*v1*" "*boost*" -o coverage-filtered.info 
genhtml -o coverage.html ./coverage-filtered.info
lcov -d "." -z
```