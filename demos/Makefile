default:
	$(MAKE) -C MAP-Elites-Arm/
	$(MAKE) -C NK/
	$(MAKE) -C SpatialCoop2017/

clean:
	$(MAKE) clean -C MAP-Elites-Arm/
	$(MAKE) clean -C NK/
	$(MAKE) clean -C SpatialCoop2017/
	$(MAKE) clean -C Sudoku/

debug:
	$(MAKE) debug -C MAP-Elites-Arm/
	$(MAKE) debug -C NK/
	$(MAKE) debug -C SpatialCoop2017/

web-test:
	$(MAKE) web -C MAP-Elites-Arm/
	$(MAKE) web -C SpatialCoop2017/
	$(MAKE) web -C Sudoku/

native-test: default

native-test-debug: debug
