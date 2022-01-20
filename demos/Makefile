default:
	# $(MAKE) -C EasyChair/
	$(MAKE) -C MAP-Elites-Arm/
	$(MAKE) -C NK/
	# $(MAKE) -C SelectionAnalyze/
	$(MAKE) -C SpatialCoop2017/
	# $(MAKE) -C utils/

clean:
	# $(MAKE) clean -C EasyChair/
	$(MAKE) clean -C MAP-Elites-Arm/
	$(MAKE) clean -C NK/
	$(MAKE) clean -C SelectionAnalyze/
	$(MAKE) clean -C SpatialCoop2017/
	$(MAKE) clean -C Sudoku/
	# $(MAKE) clean -C utils/

debug:
	# $(MAKE) debug -C EasyChair/
	$(MAKE) debug -C MAP-Elites-Arm/
	$(MAKE) debug -C NK/
	# $(MAKE) debug -C SelectionAnalyze/
	$(MAKE) debug -C SpatialCoop2017/
	# $(MAKE) debug -C utils/

web-test:
	# $(MAKE) web -C EasyChair/
	$(MAKE) web -C MAP-Elites-Arm/
	# $(MAKE) web -C NK/
	# $(MAKE) web -C SelectionAnalyze/
	$(MAKE) web -C SpatialCoop2017/
	$(MAKE) web -C Sudoku/
	# $(MAKE) web -C utils/

native-test: default

native-test-debug: debug
