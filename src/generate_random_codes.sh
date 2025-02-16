#!/bin/bash

# Default parameters
NUM_SEEDS=100
CODE_DIR="compilable_codes"
DEBUG_DIR="debug_binaries"
NODEBUG_DIR="nodebug_binaries"
DIFF_DIR="diff_output"

# Create directories
mkdir -p "$CODE_DIR" "$DEBUG_DIR" "$NODEBUG_DIR" "$DIFF_DIR"

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
        continue
    fi
    echo "Successfully compiled (with debug info): code_$i.c"

    # Compile without debugging information
    gcc -o "$NODEBUG_DIR/code_$i" "$CODE_DIR/code_$i.c"
    if [ $? -ne 0 ]; then
        echo "Compilation failed (without debug info): code_$i.c"
        continue
    fi
    echo "Successfully compiled (without debug info): code_$i.c"

    # Compare the two binaries
    objdump --disassemble --section=.text "$DEBUG_DIR/code_$i" > "$DEBUG_DIR/code_$i.txt"
    objdump --disassemble --section=.text "$NODEBUG_DIR/code_$i" > "$NODEBUG_DIR/code_$i.txt"
    
    #remove the 2nd line of the objdump output
    sed -i '2d' "$DEBUG_DIR/code_$i.txt"
    sed -i '2d' "$NODEBUG_DIR/code_$i.txt"

    # save diff_output to DIFF_DIR
    diff "$DEBUG_DIR/code_$i.txt" "$NODEBUG_DIR/code_$i.txt" > "$DIFF_DIR/code_$i.txt"

    # Check if there are differences
    if [ $? -ne 0 ]; then
        echo "Differences found between code_$i with and without debug info." >> failed_numbers.txt
    else
        echo "No differences for code_$i." >> successful_numbers.txt
    fi
    # Remove the binaries
    rm "$DEBUG_DIR/code_$i" "$NODEBUG_DIR/code_$i"

done

echo "Done"