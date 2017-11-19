export LOGLVL = 2

all:
	+$(MAKE) -C logger
	+$(MAKE) -C src_omega
	+$(MAKE) -C src_host

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.exe" -type f -delete
	find . -name "*.txt" -type f -delete