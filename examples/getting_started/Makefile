LIST = hello_world calling_js_from_cpp exchanging_data passing_to_js \
		callable_cpp_funcs js_take_charge

html_targets = $(addsuffix .html, $(LIST))
js_targets = $(addsuffix .js, $(LIST))

all: $(LIST)
	@echo "Built all examples!"

hello_world: hello_world.cpp
	emcc hello_world.cpp -o hello_world.html

calling_js_from_cpp: calling_js_from_cpp.cpp
	emcc calling_js_from_cpp.cpp -o calling_js_from_cpp.html

exchanging_data: exchanging_data.cpp
	emcc exchanging_data.cpp -o exchanging_data.html

passing_to_js: passing_to_js.cpp
	emcc passing_to_js.cpp -o passing_to_js.html

callable_cpp_funcs: callable_cpp_funcs.cpp
	emcc -s EXPORTED_FUNCTIONS="['_SquareVal', '_main']" \
		callable_cpp_funcs.cpp -o callable_cpp_funcs.html

js_take_charge: js_take_charge.cpp
	emcc -s EXPORTED_FUNCTIONS="['_SquareVal']" js_take_charge.cpp \
		-o js_take_charge.js
	cp ../data/js_take_charge.html js_take_charge.html

clean:
	rm -f $(html_targets)
	rm -f $(js_targets)
