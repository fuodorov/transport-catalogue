.DEFAULT_GOAL := help
.PHONY: deps lint format help

REPO_ROOT:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

build: ## build program
	mkdir -p build
	cd build && cmake ../src && cmake --build .

format: ## autoformat code with clang-format
	clang-format -i src/*.cpp src/*.h src/*/*.cpp src/*/*.h -style=Google

deps: ## install dependencies
	sudo apt install -y clang-format 
	sudo apt install -y libtbb-dev

run: ## run
	./build/transport_catalogue make_base < data/input/make_base.json
	./build/transport_catalogue process_requests < data/input/process_requests.json > data/output/process_requests.json

help: ## Show help message
	@grep -E '^[a-zA-Z0-9 -]+:.*#'  Makefile | sort | while read -r l; do printf "\033[1;32m$$(echo $$l | cut -f 1 -d':')\033[00m:$$(echo $$l | cut -f 2- -d'#')\n"; done