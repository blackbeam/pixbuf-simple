all: build/default/pixbuf.node

test: test/test.js all
	nodeunit test/test.js

build/default/pixbuf.node: src/Pixbuf.cc src/Pixbuf.hh wscript
	node-waf -v configure build

clean:
	node-waf distclean
