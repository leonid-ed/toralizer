.DEFAULT_GOAL := help

shared_lib := toralize.so
src := toralize.c toralize.h connect_test.c

.PHONY: reformat # Run clang-format to reformat the source files
reformat: $(scr)
	clang-format -i $(src)

.PHONY: build_connect_test # Build connect_test
build_connect_test: connect_test.c
	$(CC) connect_test.c -Wall -Wextra -pedantic -pedantic-errors -o connect_test

.PHONY: build # Build the shared library
build: toralize.c toralize.h
	$(CC) toralize.c -shared -ldl -fPIC -Wall -Wextra -errors -o $(shared_lib)

.PHONY: clean # Clean the building directory
clean:
	rm -rf $(shared_lib)
	rm -rf connect_test

.PHONY: help # Generates list of targets with descriptions
help:
	@grep '^.PHONY: .* #' Makefile | sort | sed 's/\.PHONY: \(.*\) # \(.*\)/\1\#\2/' | column -t -s#
