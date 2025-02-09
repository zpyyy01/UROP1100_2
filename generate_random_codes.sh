#!/bin/bash

# Default parameters
NUM_SEEDS=20
CODE_DIR="compilable_codes"
DEBUG_DIR="debug_binaries"
NODEBUG_DIR="nodebug_binaries"

# Create directories
mkdir -p "$CODE_DIR" "$DEBUG_DIR" "$NODEBUG_DIR"

# Clear the success and failure record files
> successful_numbers.txt
> failed_numbers.txt

for ((i = 1; i <= NUM_SEEDS; i++))
do

    SEED=$RANDOM

    # Generate C code
    csmith --seed $SEED --main --output "$CODE_DIR/code_$i.c"

    # Compile with debugging information
    gcc -g -o "$DEBUG_DIR/code_$i" "$CODE_DIR/code_$i.c"
    if [ $? -ne 0 ]; then
        echo "Compilation failed (with debug info): code_$i.c"
        echo $i >> failed_numbers.txt
        continue
    fi
    echo "Successfully compiled (with debug info): code_$i.c"

    # Compile without debugging information
    gcc -o "$NODEBUG_DIR/code_$i" "$CODE_DIR/code_$i.c"
    if [ $? -ne 0 ]; then
        echo "Compilation failed (without debug info): code_$i.c"
        echo $i >> failed_numbers.txt
        continue
    fi
    echo "Successfully compiled (without debug info): code_$i.c"

    # Compare the two binaries
    if diff -q "$DEBUG_DIR/code_$i" "$NODEBUG_DIR/code_$i" > /dev/null; then
        echo "Binaries are the same: code_$i"
    else
        echo "Binaries are different: code_$i"
    fi
    # Record the successful file number
    echo $i >> successful_numbers.txt
done

echo "Done"