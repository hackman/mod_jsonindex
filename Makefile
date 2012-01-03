APXS=/usr/local/apache/bin/apxs
APXS2=/usr/local/apache2/bin/apxs
MODULE=mod_jsonindex.c

ap13:
	sudo $(APXS) -c $(MODULE)
ap2:
	sudo $(APXS2) -c $(MODULE)
all:
	ap13

clean:
	sudo rm -rf .libs *.o *.lo *.so *.la *.a *.slo 

install:
	sudo $(APXS) -cia -n hive $(MODULE)

