all: build/Release/pixbuf.node

test: test/test.js all
	./node_modules/nodeunit/bin/nodeunit test/test.js

debug-test: build/Debug/pixbuf.node
	valgrind --trace-children=yes ./node_modules/nodeunit/bin/nodeunit test/test.js

build/Release/pixbuf.node: src/pixbuf.cc src/pixbuf.hh binding.gyp package.json
	npm install

build/Debug/pixbuf.node: src/pixbuf.cc src/pixbuf.hh binding.gyp package.json
	npm run-script install-debug

clean:
	npm run-script clean
