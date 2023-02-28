# Data Management Tools

This directory contains a set of tools for managing more or less genetic data.


## Individual pieces of data

* Datum.hpp - emp::Datum holds a single value, which can be a string or a float.  It shifts
  between these types as needed.


## Tracking series of Data

* DataNode.hpp - Manage a stream of data of a specific type; can specify at compile time how
  data should be handled (tracking averages, modes, entropy, etc.)  Can also be made to
  pull data when needed.

* DataInterface.hpp - A generic interface to DataNodes to make the easy to manage collectively.

* DataManager.hpp - Manages a collection of DataNodes that all have the same settings.

* DataFile.hpp - A collection of DataNodes that automatically output desired information to
  an output file.

* DataLog.hpp - Manage a series of data, tracking calculations and printing histograms.


## Tracking arbitrary named data

* DataMap.hpp - Links variable names to arbitrary type values, these are stored in a single
  memory block for locality and easy group copying.

* DataLayout.hpp - Keeps track of information associated with each variable in a data map,
  including location, type, description, etc.

* MemoryImage.hpp - Block of memory managed using a given DataLayout.

* AnnotatedType.hpp - Base class for objects that have a linked DataMap.

* VarMap.hpp - Similar to DataMap, but types are stored with variables and not consecutive
  in memory.

* DataMapParser.hpp - A parser to take an equation based on variables in a DataLayout that
  will produce a lambda.  If a DataMap is passed into the lambda the equation will be
  calculated and the result returned.

* Trait.hpp - ?


## To add?

DataFrame - rows are entries, columns are types, stored by column for fast calculation.
DataColumn = vector from DataFrame with type information.
DataRow - Same interface as DataMap; refers to associated DataFrame.

DataTracker - Handles all of the functionality of DataNode, DataLog, etc., but more dynamic
  using lambdas to deal with values as needed.


## To modify?

Datum - should be able to do uint64_t?