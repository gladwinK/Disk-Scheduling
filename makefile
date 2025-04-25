build:
	g++ ./DiskSchedulling\ Algos/calculateMetrics.cpp ./DiskSchedulling\ Algos/clook.cpp ./DiskSchedulling\ Algos/cscan.cpp ./DiskSchedulling\ Algos/fcfs.cpp ./DiskSchedulling\ Algos/hdsa.cpp ./InputOutput/InputOutput.cpp ./DiskSchedulling\ Algos/look.cpp main.cpp ./QueueGeneration/QueueGeneration.cpp ./DiskSchedulling\ Algos/scan.cpp ./DiskSchedulling\ Algos/sstf.cpp -w -o main 
	@echo "Build complete. Executable is 'main'."
run:
	./main

clean:
	rm -f main
	@echo "Cleaned up the build files."