BUILD_DIR=/usr/local/apache/build/
APXS=/usr/local/apache/bin/apxs
MODULE=mod_jsonindex.c

all:
	sudo $(APXS) -c $(MODULE)

clean:
	sudo rm -rf .libs *.o *.lo *.so *.la *.a *.slo 

install:
	sudo $(APXS) -cia -n hive $(MODULE)

