import os
import subprocess
import random

# Default parameters
NUM_SEEDS = 100
CODE_DIR = "compilable_codes"
DEBUG_DIR = "debug_binaries"
NODEBUG_DIR = "nodebug_binaries"
DIFF_DIR = "diff_output"

# Create directories
os.makedirs(CODE_DIR, exist_ok=True)
os.makedirs(DEBUG_DIR, exist_ok=True)
os.makedirs(NODEBUG_DIR, exist_ok=True)
os.makedirs(DIFF_DIR, exist_ok=True)

# Clear the success and failure record files
with open("successful_numbers.txt", "w"), open("failed_numbers.txt", "w"):
    pass

for i in range(1, NUM_SEEDS + 1):
    seed = random.randint(0, 2**32 - 1)

    # Generate C code
    subprocess.run(["csmith", "--seed", str(seed), "--main", "--output", f"{CODE_DIR}/code_{i}.c"])

    # Compile with debugging information
    debug_compilation = subprocess.run(
        ["gcc", "-g", "-o", f"{DEBUG_DIR}/code_{i}", f"{CODE_DIR}/code_{i}.c"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if debug_compilation.returncode != 0:
        print(f"Compilation failed (with debug info): code_{i}.c")
        continue
    print(f"Successfully compiled (with debug info): code_{i}.c")

    # Compile without debugging information
    nodebug_compilation = subprocess.run(
        ["gcc", "-o", f"{NODEBUG_DIR}/code_{i}", f"{CODE_DIR}/code_{i}.c"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if nodebug_compilation.returncode != 0:
        print(f"Compilation failed (without debug info): code_{i}.c")
        continue
    print(f"Successfully compiled (without debug info): code_{i}.c")

    # Compare the two binaries
    subprocess.run(["objdump", "--disassemble", "--section=.text", f"{DEBUG_DIR}/code_{i}"],
                   stdout=open(f"{DEBUG_DIR}/code_{i}.txt", "w"))
    subprocess.run(["objdump", "--disassemble", "--section=.text", f"{NODEBUG_DIR}/code_{i}"],
                   stdout=open(f"{NODEBUG_DIR}/code_{i}.txt", "w"))

    # Remove the 2nd line of the objdump output
    for dir_path in [DEBUG_DIR, NODEBUG_DIR]:
        with open(f"{dir_path}/code_{i}.txt", "r") as file:
            lines = file.readlines()
        with open(f"{dir_path}/code_{i}.txt", "w") as file:
            file.writelines(lines[:1] + lines[2:])  # Skip the second line

    # Save diff_output to DIFF_DIR
    diff = subprocess.run(
        ["diff", f"{DEBUG_DIR}/code_{i}.txt", f"{NODEBUG_DIR}/code_{i}.txt"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )

    # Check if there are differences
    if diff.returncode != 0:
        with open("failed_numbers.txt", "a") as f:
            f.write(f"Differences found between code_{i} with and without debug info.\n")
    else:
        with open("successful_numbers.txt", "a") as f:
            f.write(f"No differences for code_{i}.\n")

    # Remove the binaries
    os.remove(f"{DEBUG_DIR}/code_{i}")
    os.remove(f"{NODEBUG_DIR}/code_{i}")

print("Done")