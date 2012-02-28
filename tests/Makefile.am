
BASE_TESTS = test.conf test.conf1 test1.conf

pkgdata_DATA := $(wildcard *.conf) run-tests.sh

run-test.sh :
	chmod +x run-tests.sh

check :
	$(SHELL) run-tests.sh $(BASE_TESTS)
