# =============================================================================
# Makefile  (lives in tests/)
# =============================================================================
#
# Targets:
#   make test_path      — build & run BFS / path tests
#   make test_deviation — build & run deviation classification tests
#   make test_graph     — build & run graph integration test (needs network)
#   make test_challenge_generator — build & run challenge generator integration test
#   make tests          — run all four suites in sequence
#   make clean          — remove compiled binaries
# =============================================================================

CC      = gcc
CFLAGS  = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -g

.DEFAULT_GOAL := tests

SRCDIR  = ..

# Headers are in SRCDIR; -I.. lets every #include "../foo.h" also work as
# #include "foo.h" inside the source files that use the latter style.
INCLUDES = -I$(SRCDIR)

COMMON_SRC = $(SRCDIR)/graph.c $(SRCDIR)/cJSON.c

# --------------------------------------------------------------------------
# test_path — in-memory graph, no network, no curl
# --------------------------------------------------------------------------
test_path: test_path.c $(SRCDIR)/path.c $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_path \
	    test_path.c $(SRCDIR)/path.c $(COMMON_SRC)
	@echo ""
	@echo "Running test_path..."
	@echo "--------------------"
	./test_path

# --------------------------------------------------------------------------
# test_deviation — in-memory graph, no network, no curl
# --------------------------------------------------------------------------
test_deviation: test_deviation.c $(SRCDIR)/deviation.c $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_deviation \
	    test_deviation.c $(SRCDIR)/deviation.c $(COMMON_SRC)
	@echo ""
	@echo "Running test_deviation..."
	@echo "-------------------------"
	./test_deviation

# --------------------------------------------------------------------------
# test_graph — hits the Overpass API (or local cache); needs libcurl
# --------------------------------------------------------------------------
test_graph: test_graph.c $(SRCDIR)/fetcher.c $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_graph \
	    test_graph.c $(SRCDIR)/fetcher.c $(COMMON_SRC) -lcurl
	@echo ""
	@echo "Running test_graph..."
	@echo "---------------------"
	cd $(SRCDIR) && ./tests/test_graph

# --------------------------------------------------------------------------
# test_challenge_generator — integration with real Catalunya comarca graph
# --------------------------------------------------------------------------
test_challenge_generator: test_challenge_generator.c $(SRCDIR)/challenge_generator.c $(SRCDIR)/path.c $(SRCDIR)/fetcher.c $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_challenge_generator \
	    test_challenge_generator.c $(SRCDIR)/challenge_generator.c $(SRCDIR)/path.c $(SRCDIR)/fetcher.c $(COMMON_SRC) -lcurl
	@echo ""
	@echo "Running test_challenge_generator..."
	@echo "-----------------------------------"
	cd $(SRCDIR) && ./tests/test_challenge_generator

# --------------------------------------------------------------------------
tests: test_path test_deviation test_graph test_challenge_generator

test: tests

clean:
	rm -f test_path test_deviation test_graph test_challenge_generator

.PHONY: test_path test_deviation test_graph test_challenge_generator tests test clean
