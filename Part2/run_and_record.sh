OUTPUT_FILE="part2output.txt"

echo "Compiling monitor.c and sleepingStylistMon.c..."
gcc -Wall -pthread -c monitor.c
if [ $? -ne 0 ]; then
    echo "Compilation of monitor.c failed. Exiting."
    exit 1
fi

gcc -Wall -pthread -o sleepingStylistMon sleepingStylistMon.c monitor.o
if [ $? -ne 0 ]; then
    echo "Compilation of sleepingStylistMon.c failed. Exiting."
    exit 1
fi

echo "Compilation successful!"
echo "Running sleepingStylistMon... output saved to $OUTPUT_FILE"
echo ""

# Run the program and save output to file AND display it on screen
./sleepingStylistMon 2>&1 | tee "$OUTPUT_FILE"

echo ""
echo "Done! Output saved to $OUTPUT_FILE"