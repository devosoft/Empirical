default:
	$(MAKE) -C base/
	$(MAKE) -C control/
	$(MAKE) -C data/
	$(MAKE) -C Evolve/
	$(MAKE) -C geometry/
	$(MAKE) -C hardware/
	$(MAKE) -C meta/
	$(MAKE) -C prefab/
	$(MAKE) -C ProjectTemplate/
	$(MAKE) -C tools/
	$(MAKE) -C web/

clean:
	$(MAKE) clean -C base/
	$(MAKE) clean -C control/
	$(MAKE) clean -C data/
	$(MAKE) clean -C Evolve/
	$(MAKE) clean -C geometry/
	$(MAKE) clean -C hardware/
	$(MAKE) clean -C meta/
	$(MAKE) clean -C prefab/
	$(MAKE) clean -C ProjectTemplate/
	$(MAKE) clean -C tools/
	$(MAKE) clean -C web/

debug:
	$(MAKE) debug -C base/
	$(MAKE) debug -C control/
	$(MAKE) debug -C data/
	$(MAKE) debug -C Evolve/
	$(MAKE) debug -C geometry/
	$(MAKE) debug -C hardware/
	$(MAKE) debug -C meta/
	$(MAKE) debug -C prefab/
	$(MAKE) debug -C ProjectTemplate/
	$(MAKE) debug -C tools/
	$(MAKE) debug -C web/

web-test:
	$(MAKE) debug -C prefab/
	$(MAKE) -C prefab/
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
