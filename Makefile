all: build/default/pixbuf.node

test: test/test.js all
	nodeunit test/test.js

build/default/pixbuf.node: Pixbuf.cc Pixbuf.hh build/c4che/build.config.py
	node-waf -v build

build/c4che/build.config.py: wscript
	node-waf -v configure

clear:
	rm -rf build
