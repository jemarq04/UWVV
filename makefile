RUFF      := $(shell which ruff 2>/dev/null || echo uvx ruff)
PRECOMMIT := $(shell which pre-commit 2>/dev/null || echo uvx pre-commit)

lint:
	$(PRECOMMIT) run -a

clang-format:
	find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\)' | xargs clang-format -i

ruff-format:
	$(RUFF) format
