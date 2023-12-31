SUBDIRS := shared_libraries euphoric_tools osdk

all install clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C "$$d" $(MAKECMDGOALS) || exit $?; \
	done

