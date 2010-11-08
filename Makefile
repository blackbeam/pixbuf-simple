all: build/default/pixbuf.node

build/default/pixbuf.node: Pixbuf.cc Pixbuf.hh build/c4che/build.config.py
	node-waf -v build

build/c4che/build.config.py:
	node-waf -v configure

