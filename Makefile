.PHONY: all

all: .build/Makefile
	$(MAKE) -C .build

.build/Makefile:
	mkdir -p .build
	cd .build; cmake ..
