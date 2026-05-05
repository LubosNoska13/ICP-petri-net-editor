CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g

BUILD_DIR = build
SRCS = $(wildcard *.cpp)

.PHONY: all clean

all:
	@mkdir -p $(BUILD_DIR)
	@failed=0; \
	for src in $(SRCS); do \
		obj="$(BUILD_DIR)/$${src%.cpp}.o"; \
		printf "Preklad $$src ... "; \
		if $(CXX) $(CXXFLAGS) -c "$$src" -o "$$obj"; then \
			echo "OK"; \
		else \
			echo "NEPODARILO SA"; \
			failed=1; \
		fi; \
	done; \
	if [ $$failed -eq 0 ]; then \
		echo "Všetky súbory sa úspešne preložili."; \
	else \
		echo "Niektoré súbory sa nepreložili."; \
		exit 1; \
	fi

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Vyčistené."