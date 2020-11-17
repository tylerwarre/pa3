Our program can be compiled ussing GCC with the following command: gcc pa3.c -lm

To run our program you must supply the executable, input file, schedule type, and scheduler mode (optional)
Ex. "./a.out input1.txt EDF EE"
Ex. "./a.out input2.txt RM"

An output file will automatically be generated based on the command line arguments
Ex. "./a.out input1.txt EDF EE" -> EDF_EE_input1.txt.log
Ex. "./a.out input2.txt RM" -> RM_input2.txt.log