.DEFAULT_GOAL := help

shared_lib := toralize.dylib
src := toralize.c toralize.h connect_test.c getaddrinfo_example.c

.PHONY: reformat # Run clang-format to reformat the source files
reformat: $(scr)
	clang-format -i $(src)

.PHONY: build_getaddrinfo_example # Build connect_test
build_getaddrinfo_example: getaddrinfo_example.c
	$(CC) getaddrinfo_example.c -Wall -Wextra -pedantic -pedantic-errors -o connect_test

.PHONY: build_connect_test # Build connect_test
build_connect_test: connect_test.c
	$(CC) connect_test.c -Wall -Wextra -pedantic -pedantic-errors -o connect_test

.PHONY: build # Build the shared library
build: toralize.c toralize.h
	$(CC) toralize.c -dynamiclib -ldl -Wall -Wextra -pedantic -pedantic-errors -o $(shared_lib)

.PHONY: clean # Clean the building directory
clean:
	rm -rf $(shared_lib) $(shared_lib).dSYM
	rm -rf connect_test connect_test.dSYM
	rm -rf getaddrinfo_example

.PHONY: help # Generates list of targets with descriptions
help:
	@grep '^.PHONY: .* #' Makefile | sort | sed 's/\.PHONY: \(.*\) # \(.*\)/\1\#\2/' | column -t -s#
