#!/bin/bash

# Prompt user for number of iterations
echo "Enter the number of iterations: "
read iterations

# Loop through iterations
for (( i=1; i<=$iterations; i++ ))
do
echo "Running iterationg $i "
  # Call anthills command with quiet and csv options
  ./anthills quiet csv
  
  # Rename output file with iteration number
  mv output.csv output_$i.csv
done

echo "Done!"
