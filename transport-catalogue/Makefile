.DEFAULT_GOAL := help
.PHONY: deps lint format help

REPO_ROOT:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

compile: ## compile c++ code
	g++ *.cpp */*.cpp -Wall -Werror -std=c++17 -ltbb -lpthread -o main

format: ## autoformat code with clang-format
	clang-format -i *.cpp */*.cpp *.h */*.h

deps: ## install dependencies
	sudo apt install -y clang-format
	sudo apt install -y libtbb-dev

run: ## run program
	./main

help: ## Show help message
	@grep -E '^[a-zA-Z0-9 -]+:.*#'  Makefile | sort | while read -r l; do printf "\033[1;32m$$(echo $$l | cut -f 1 -d':')\033[00m:$$(echo $$l | cut -f 2- -d'#')\n"; done