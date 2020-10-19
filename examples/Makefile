default:
	$(MAKE) -C base/
	$(MAKE) -C control/
	$(MAKE) -C data/
	$(MAKE) -C Evolve/
	$(MAKE) -C geometry/
	$(MAKE) -C hardware/
	$(MAKE) -C meta/
	$(MAKE) -C ProjectTemplate/
	$(MAKE) -C tools/
	$(MAKE) -C web/
	$(MAKE) -C bits/
	$(MAKE) -C compiler/
	$(MAKE) -C datastructs/
	$(MAKE) -C io/
	$(MAKE) -C math/
	$(MAKE) -C testing/

clean:
	$(MAKE) clean -C base/
	$(MAKE) clean -C control/
	$(MAKE) clean -C data/
	$(MAKE) clean -C Evolve/
	$(MAKE) clean -C geometry/
	$(MAKE) clean -C hardware/
	$(MAKE) clean -C meta/
	$(MAKE) clean -C ProjectTemplate/
	$(MAKE) clean -C tools/
	$(MAKE) clean -C web/
	$(MAKE) clean -C bits/
	$(MAKE) clean -C compiler/
	$(MAKE) clean -C datastructs/
	$(MAKE) clean -C io/
	$(MAKE) clean -C math/
	$(MAKE) clean -C testing/

debug:
	$(MAKE) debug -C base/
	$(MAKE) debug -C control/
	$(MAKE) debug -C data/
	$(MAKE) debug -C Evolve/
	$(MAKE) debug -C geometry/
	$(MAKE) debug -C hardware/
	$(MAKE) debug -C meta/
	$(MAKE) debug -C ProjectTemplate/
	$(MAKE) debug -C tools/
	$(MAKE) debug -C web/
	$(MAKE) debug -C bits/
	$(MAKE) debug -C compiler/
	$(MAKE) debug -C datastructs/
	$(MAKE) debug -C io/
	$(MAKE) debug -C math/
	$(MAKE) debug -C testing/

web-test:
	$(MAKE) debug -C web/
	$(MAKE) -C web/

native-test:
	$(MAKE) -C base/
	$(MAKE) -C control/
	$(MAKE) -C data/
	$(MAKE) -C Evolve/
	$(MAKE) -C geometry/
	$(MAKE) -C hardware/
	$(MAKE) -C meta/
	$(MAKE) -C ProjectTemplate/
	$(MAKE) -C tools/
	$(MAKE) -C bits/
	$(MAKE) -C compiler/
	$(MAKE) -C datastructs/
	$(MAKE) -C io/
	$(MAKE) -C math/
	$(MAKE) -C testing/

native-test-debug:
	$(MAKE) debug -C base/
	$(MAKE) debug -C control/
	$(MAKE) debug -C data/
	$(MAKE) debug -C Evolve/
	$(MAKE) debug -C geometry/
	$(MAKE) debug -C hardware/
	$(MAKE) debug -C meta/
	$(MAKE) debug -C ProjectTemplate/
	$(MAKE) debug -C tools/
	$(MAKE) debug -C bits/
	$(MAKE) debug -C compiler/
	$(MAKE) debug -C datastructs/
	$(MAKE) debug -C io/
	$(MAKE) debug -C math/
	$(MAKE) debug -C testing/
