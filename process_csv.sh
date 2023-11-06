#!/bin/bash

# Check if the user provided a CSV file as an argument
if [ $# -ne 1 ]; then
  echo "Usage: $0 <csv_file>"
  exit 1
fi

# Store the CSV file path provided as an argument
csv_file="$1"

# Check if the provided file exists
if [ ! -f "$csv_file" ]; then
  echo "Error: CSV file not found: $csv_file"
  exit 1
fi

# Call the C++ program with the CSV file as an argument
g++ -o delay_calculator delay_calculator.cpp 
./delay_calculator "$csv_file"
