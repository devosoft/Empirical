# Data Collection and Recording Tools

Empirical includes a variety of tools for gathering and recording data.
The core of these tools is the DataNode class. DataNodes are containers
that you can pass as much data as you like into. When DataNodes are
built, they can be given various modifiers (specified as template
arguments) which control how much information they will collect about
the data they are passed. For instance, the `data::Current` modifier
gives the DataNode the power to remember the last value it was passed,
whereas the `data::Stats` modifier keeps track of a variety of
statistics about the distribution of data that the node has been
passed. Except where otherwise noted, modifiers can be combined freely.
Some also have dependencies on simpler modifiers. On the whole,
DataNodes are designed to be as light-weight as possible while still
keeping track of the desired information.

DataNodes that accept the same type of data and have the same modifiers
can be grouped together using a DataManager.

The DataInterface class provides a general interface for interacting
with DataNodes of any type. This is useful in cases where you have a
collection of different types of DataNodes and want to operate on them
without casting.

The DataFile class provides an interface for recording data at regular
intervals. This data can come from DataNodes, the return of a specified
function, or the contents of a specified variable. DataFiles are useful
for collecting data over the course of a computational experiment.

## Data Tools API

### DataNodes

```{eval-rst}
.. doxygenfile:: emp/data/DataNode.hpp
  :project: Empirical
  :no-link:
```

### DataManagers

```{eval-rst}
.. doxygenfile:: emp/data/DataManager.hpp
  :project: Empirical
  :no-link:
```

### DataInterfaces

```{eval-rst}
.. doxygenfile:: emp/data/DataInterface.hpp
  :project: Empirical
  :no-link:
```

### DataFiles

```{eval-rst}
.. doxygenfile:: emp/data/DataFile.hpp
  :project: Empirical
  :no-link:
```

