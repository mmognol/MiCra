      
# Compiler for DPU
CC := dpu-upmem-dpurte-clang

# Compiler Flags for DPU C++
# Renamed FLAGSP to DPU_CXXFLAGS for clarity and convention
DPU_CXXFLAGS := -pg -std=gnu++20 -O3 -fno-builtin -DNR_TASKLETS=16 -fno-rtti -fno-exceptions

# Source Files
SRC_MAIN := heapsort.cpp
SRC_NOCACHE := heapsort_nocache.cpp

# Target DPU Binary Names
TARGET_MAIN := heapsort.dpu
# Give the nocache version a distinct name
TARGET_NOCACHE := heapsort_nocache.dpu

# Phony targets (targets that are not actual files)
.PHONY: all clean nocache

# Default target: build the standard version
all: $(TARGET_MAIN) $(TARGET_NOCACHE)

# Target to build the no-cache version
nocache: $(TARGET_NOCACHE)

# Target to clean up generated files
clean:
	# Use -f to suppress errors if files don't exist
	rm -f $(TARGET_MAIN) $(TARGET_NOCACHE) *.o *.dpu *.dpu.o

# Rule to build the standard DPU binary
$(TARGET_MAIN): $(SRC_MAIN)
	@echo "Compiling $@ from $<..."  # Optional: show what's being compiled
	$(CC) $(DPU_CXXFLAGS) $^ -o $@

# Rule to build the no-cache DPU binary
$(TARGET_NOCACHE): $(SRC_NOCACHE)
	@echo "Compiling $@ from $<..."  # Optional: show what's being compiled
	$(CC) $(DPU_CXXFLAGS) $^ -o $@

    