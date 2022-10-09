PAGE_SRCS := $(shell find -name '*.[0-9].txt' -printf '%P\n')
PAGES := $(patsubst %.txt,output/%,$(PAGE_SRCS))

all: $(PAGES)

output/%: %.txt
	@mkdir -p $$(dirname '$@')
	@echo [GEN] $@ ; manscript -o $@ $<

clean:
	rm -rf output
