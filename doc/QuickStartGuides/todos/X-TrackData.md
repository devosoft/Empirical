# Data Tracking

Overview of data tracking:
1. Make a `emp::Ptr<DataMonitor<type, structure>>` where your data will be stored in private
2. Make public method `SetupMyDataFile`, which sets up the file and plugs the node into the file for whatever stats you want
3. Make public method `GetMyDataNode`, which instantiates you DataMonitor (special type of DataNode) and sets an update
trigger to collect the data you want to track
4. Make public method `CalcMyVal` which grabs the data from an individual organism and does any necessary processing before it is put
in the data node
5. Set up the files in your .cc file
6. Test
7. Prosper

To track data in your new world:


in private:
```cpp
emp::Ptr<DataMonitor<double, emp::<data::Histogram>>> my_data_node; // double and Histogram can be replaced by what you need
/*
Current modifiers (found in include/emp/data/DataNode.h):
    Current, Info, Log, Archive, Range, FullRange,
    Histogram, Pull, SignalReset, SignalData,
    SignalDatum, SignalRange, SignalLimits
*/
```
in public:

```cpp
World_file & SetupMyDataFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetMyDataNode();
    // Histogram needs bins set up, most others don't need anything
    node.SetupBins(-1.0, 1.0, 20);
    // Update will be the first column
    file.AddVar(update, "update", "Update");
    // Mean of my interaction value will be calculated and put in the second column automagically
    file.AddMean(node, "mean_intval", "Average org int val");
    //Histograms need every bin set up, most others don't need anything else
    file.AddHistBin(node, 0, "[Hist]()-1", "Count for hist bin -1");

    /* Current options for Add: (Found in include/emp/data/DataFile.h)
        Add, AddFun, AddVar, AddCurrent, AddMean,
        AddTotal, AddMin, AddMax, AddHistBin, AddInferiority
    */

    file.PrintHeaderKeys();
}
```

```cpp
DataMonitor<double> GetMyDataNode() {
    if (!my_data_node) {
        my_data_node.New();
        OnUpdate(
            [this](size_t) {
                my_data_node->Reset();
                for (size_t i = 0; i < pop.size(); i++) {
                    if (IsOccupied(i)) my_data_node->AddDatum(CalcMyVal(i));
                }
            }
        );

    }
    return *my_data_node;
}
```

```cpp
double CalcMyVal(size_t i) {
    return pop[i]->GetMyVal(); // Defined in your organism class
}
```

In your `project_name.cc` file in main:

```cpp
//Timing repeat for how often you want data printed
my_world.SetupPopulationFile().SetTimingRepeat(10);

/* Lots of premade options for files (found in include/emp/Evolve/World.h)
    SetupFile, setupFitnessFile,
    SetupSystematicsFile, SetupPopulationFile
*/

my_world.SetupMyDataFile().SetTimingRepeat(10); //Use your custom built stat tracker if you'd like
```
