all: hello-world calling-js-from-cpp exchanging-data passing-to-js \
		callable-cpp-funcs
	@echo "Built all examples!"

hello-world: hello_world.cpp
	emcc hello_world.cpp -o hello_world.html

calling-js-from-cpp: calling_js_from_cpp.cpp
	emcc calling_js_from_cpp.cpp -o calling_js_from_cpp.html

exchanging-data: exchanging_data.cpp
	emcc exchanging_data.cpp -o exchanging_data.html

passing-to-js: passing_to_js.cpp
	emcc passing_to_js.cpp -o passing_to_js.html

callable-cpp-funcs: callable_cpp_funcs.cpp
	emcc -s EXPORTED_FUNCTIONS="['_SquareVal', '_main']" \
		callable_cpp_funcs.cpp -o callable_cpp_funcs.html

clean:
	rm -r *.html
	rm -r *.js
