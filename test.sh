if [ "$1" -eq "1" ]; then
    file=examples/example_simple.c
    elf=build/example_simple.elf
elif [ "$1" -eq "2" ]; then
    file=examples/example_subcommand.c
    elf=build/example_subcommand.elf
else 
    echo Which test to run?
    echo "Usage: $0 1 -- [ARGS]"
    exit 1
fi

echo Building...
mkdir -p build

if clang -Wall -I. -o $elf -ggdb -fsanitize=address,undefined $file; then
    true
else 
    echo Build failed.
    exit 1
fi

echo Running...
shift
./"$elf" "$@"
if [ $? -eq 1 ]; then
    echo 
    echo \"$elf\" failed.
    exit 1
fi

