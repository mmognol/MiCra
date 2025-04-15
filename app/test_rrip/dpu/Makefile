CC := dpu-upmem-dpurte-clang
FLAGSP := -std=gnu++20 -O3 -fno-builtin -DNR_TASKLETS=16 -fno-rtti -fno-exceptions

ADD_IN_VECTEUR := add_in_vecteur.dpu

SCR := main.cpp

.PHONY: all clean

all: $(ADD_IN_VECTEUR)

clean:
	rm -f $(ADD_IN_VECTEUR) *.o *.dpu *.dpu.o

$(ADD_IN_VECTEUR): $(SCR)
	$(CC) $(FLAGSP) $^ -o $@