default: web

include ../../Makefile-base.mk

JS_TARGETS := Animate.js Attributes.js assert.js Canvas.js Div.js DP.js Example.js Font.js Graph.js Hello.js Image.js keypress.js Sudoku.js Table.js TextArea.js Tween.js Web.js

debug: FLAGS_web := $(FLAGS_WEB_DEBUG)
debug: $(JS_TARGETS)

web: FLAGS_web := $(FLAGS_WEB_OPT)
web: $(JS_TARGETS)

%.js: %.cpp
	$(CXX_web) $(FLAGS_web) $< -o $@

opt: web

web-debug: debug

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
