OUTPUT_FILE="part1output.txt"
 
echo "Compiling sleepingStylistSem..."
gcc -Wall -pthread -o sleepingStylistSem sleepingStylistSem.c
 
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi
 
echo "Compilation successful!"
echo "Running sleepingStylistSem... output saved to $OUTPUT_FILE"
echo ""
 
# Run the program and save output to file AND display it on screen
./sleepingStylistSem 2>&1 | tee "$OUTPUT_FILE"
 
echo ""
echo "Done! Output saved to $OUTPUT_FILE"