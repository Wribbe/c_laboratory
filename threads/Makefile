# Makefile for c/laboratory/threads project.

srcdir := src
objectdir := objects
execdir := executables

vpath %.c $(srcdir)
vpath %.o $(objectdir)

sources := $(foreach filename,$(wildcard $(srcdir)/*.c),$(notdir $(filename)))
objects := $(sources:.c=.o)
executables := $(foreach filename,$(sources:.c=),$(execdir)/$(filename))

directories = $(srcdir) $(objectdir) $(execdir)

libraries = -lpthread
ALLFLAGS = $(libraries) $(CFLAGS)


con_mkdirs = \
	for dir in $(1); do \
		if [ ! -d $$dir ]; then \
			mkdir $$dir; \
		fi; \
	done;


.PHONY: all
all: $(executables)


%.o : %.c | dirs_exist
	$(CC) -c $(ALLFLAGS) $< -o $(objectdir)/$@


executables/%: %.o
	$(CC) $(ALLFLAGS) $(objectdir)/$< -o $@


.PHONY: dirs_exist
dirs_exist:
	@$(call con_mkdirs,$(directories))


.PHONY: clean
clean:
	rm -rf $(objectdir)
	rm -rf $(execdir)
