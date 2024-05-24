---
title: 'Empirical: A scientific software library for research, education, and public engagement'
tags:
  - C++
  - Simulation
  - Agent-based modeling
  - Emscripten
authors:
  - name: Anya Vostinar
    orcid: 0000-0001-7216-5283
    affiliation: 4
  - name: Alexander Lalejini
    orcid: 0000-0003-0994-2718
    affiliation: 8
  - name: Charles Ofria
    orcid: 0000-0003-2924-1732
    affiliation: "1, 2, 3"
  - name: Emily Dolson
    orcid: 0000-0001-8616-4898
    affiliation: "1, 2, 3"
  - name: Matthew Andres Moreno
    orcid: 0000-0003-4726-4479
    affiliation: "4, 5, 6"
affiliations:
 - name: BEACON Center for the Study of Evolution in Action, USA
   index: 1
 - name: Computer Science and Engineering, Michigan State University, USA
   index: 2
 - name: Ecology, Evolutionary Biology, and Behavior, Michigan State University, USA
   index: 3
 - name: Computer Science, Carleton College, USA
   index: 4
 - name: Ecology and Evolutionary Biology, University of Michigan, USA
   index: 5
 - name: Center for the Study of Complex Systems, University of Michigan, USA
   index: 6
 - name: Michigan Institute for Data Science, University of Michigan, USA
   index: 7
 - name: Computer Science, Grand Valley State University, USA
   index: 8
date: 13 February 2024
bibliography: paper.bib
breaks: false
---

# Summary

Empirical is a C++ library designed to promote open science and facilitate the development of scientific software that is efficient, reliable, and easily distributable to researchers and non-experts alike.
Specifically, the library sets out to fulfill the following goals:

1. **Utility:** Empirical tools streamline common scientific computing tasks such as configuration, end-to-end data management, and mathematical manipulations.
2. **Efficiency:** Empirical implements general-purpose data structures and algorithms that emphasize computational efficiency to support scientific computing workloads.
3. **Reliability:** Empirical provides sophisticated debug-mode instrumentation including audited memory management and safety-checked versions of standard library containers.
4. **Distributability:** Empirical is highly portable, uses common data formats, and facilitates compile-to-web app development with object-oriented bindings for Emscripten/WebAssembly GUI elements, all with the goal of building broadly accessible scientific software.

# Statement of Need

High quality open-science tools improve code quality, scientific rigor, and ease of replication or extension for scientific software.
Empirical's debugging suite combats C++ programming pitfalls, such as iterator invalidation, memory leakage, and out-of-bounds indexing.
Throughout, library design achieves both performance and safety through compile-time toggling of checks for undefined or incorrect behavior.

Unfortunately, in practice, scientific software is often difficult to obtain, install, or use.
Modern web-based interfaces give computational research the potential to better embody open science objectives by empowering easier and more complete access [@woelfle2011open].
Empirical leverages modern web technology to provide browser-based interactive interfaces for C++ source code.

# Empirical Features

## Better Code for Scientific Software

Empirical components are subjected to structured code review, unit testing with coverage tracking, and other best practices detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html).
Effort invested into optimization of the library's utilities enables developer-users to more easily produce safe and efficient software, especially for new developers.
We provide a [template project](https://github.com/devosoft/cookiecutter-empirical-project) that streamlines laying out crosscompilation boilerplate.

As an example of Emprical's utility, the library provides a configuration framework that includes utilities to

* create documented configuration parameters with default values in a single line of C++ code,
* adjust parameters via configuration files, command line flags, URL query parameters, or in-browser GUIs,
* perform on-the-fly configuration adjustments, and
* support independent configuration subsystems.

High-quality software needs a robust, inclusive, and diverse community of users and contributors.
Our [development practices](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html) reflect this priority.

## Realizing the Promise of Emscripten-based Web UIs

Educational editions of scientific software promote classroom learning and citizen science.
The Emscripten compiler enables an existing native codebase to additionally compile to the web [@zakai2011emscripten].
Browser-based delivery can yield particularly effective public-facing apps due to easy access and compelling interfaces.

Empirical amplifies Emscripten by fleshing out its interface for interaction with browser elements.
DOM elements are bound to corresponding C++ objects (e.g., `emp::Button` manages a `<button>` and `emp::Canvas` manages a `<canvas>`) and are easily manipulated from within C++.
Empirical also packages collections of prefabricated web widgets (e.g., configuration managers or collapsible data displays).
These tools simplify generating a mobile-friendly, web-based GUI.

A live demo of Empirical widgets, presented alongside their source C++ code, is available [here](https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo).

## Runtime Efficiency

WebAssembly's runtime efficiency --- achieving 50% to 90% of native performance [@jangda2019not] --- has driven adoption in web development [@haas2017bringing] and enabled new possibilities for browser-based scientific computation.
For example, [Avida-ED](https://avida-ed.msu.edu/) leverages WebAssembly to incorporate sophisticated agent-based evolution models into classroom activities.

More broadly, Empirical provides optimized tools for performance-critical tasks.
For example, `emp::BitArray` and `emp::BitVector` are faster drop-in replacements for their standard library equivalents (`std::bitset` and `std::vector<bool>`) with extensive additional functionality.
More fundamentally, Empirical's header-only design prioritizes ease of use and runtime performance, albeit at the cost of longer compilation times.

## Debugging

Although performant, C++'s permissiveness to out-of-bounds indexing or memory management errors can undermine the validity of generated data and analyses.
Standard library vendors --- like [`libstdc++`](https://web.archive.org/web/20210118212109/https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html), [`libc++`](https://web.archive.org/web/20210414014331/https://libcxx.llvm.org/docs/DesignDocs/DebugMode.html), and [`stl`](https://web.archive.org/web/20210121201948/https://docs.microsoft.com/en-us/cpp/standard-library/checked-iterators?view=msvc-160) --- provide some runtime safety features, but these are incomplete and poorly documented[^1].
Empirical supplements vendor offerings with debug mode stand-ins for standard library containers and even raw pointers that can identify memory leaks and invalid memory access.

Developers typically compensate for C++'s missing guardrails with external toolchains like Valgrind, GDB, and sanitizers.
Although mature, such tooling suffers substantial limitations[^1], particularly for WASM compiled with Emscripten.
Although Emscripten provides some [sanitizer support](https://web.archive.org/web/20210513071104/https://emscripten.org/docs/debugging/Sanitizers.html) and [other debugging features](https://web.archive.org/web/20210513070806/https://emscripten.org/docs/porting/Debugging.html), Empirical's safety features offset remaining limitations, such as the lack of a steppable debugger.

[^1]: For example, neither GCC 10.3 nor Clang 12.0.0 detect `std::vector` iterator invalidation when appending to a `std::vector` happens to fall within existing allocated buffer space ([GCC live example](https://perma.cc/6WDU-3C8X); [Clang live example](https://perma.cc/6SU9-CUKY)).
Clang 12.0.0's sanitizers also fail to detect this iterator invalidation ([live example](https://perma.cc/4ECQ-D5LG)).

# Outlook and Future Plans

Empirical remains under active development.
Current priorities include web-friendly refinements (e.g., file management, rich text handling) and additional step-by-step tutorials for new users.
That said, Empirical has largely converged to API stability, and releases are archived on Zenodo for those who depend on them [@empirical_2020].

Empirical already underlies major projects within digital evolution, artificial life, and genetic programming.
To benefit the broader scientific software and open science community, we look forward to welcoming new collaborations and supporting a wider collection of end-users.

# Related Software Packages

Several projects pursue objectives related to Empirical's.

### RepastHPC

RepastHPC, accessible at <https://repast.github.io/>, is a C++ modeling framework targeted to high-performance computing [@collier2013parallel; @north2013complex].
A Java-based counterpart, Repast Simphony, provides interactive GUI support.

### Boost C++ Libraries

Boost C++ Libraries, available at <https://www.boost.org/>, implement a broad portfolio of software components.
However, Boost lacks tools for web-based GUI, configuration management, or data management tailored to scientific software.

### Emscripten

Emscripten provides cross-compilation from C++ to WebAssembly and available at <https://emscripten.org/> [@zakai2011emscripten].
Empirical furnishes a complementary high-level interface to Emscripten intrinsics.

### Cheerp

Cheerp, another C++ to WebAssembly compiler, is available at <https://leaningtech.com/cheerp/>.
Like Emscripten, Cheerp provides primarily low-level APIs for browser interaction.

### Non-C++ Comparable Software

* [TinyGo](https://tinygo.org/) <!-- in-browser web interface compiler for go -->
* [WebIO](https://juliagizmos.github.io/WebIO.jl/latest/) <!-- in-browser web interface library for julia -->
* [GWT](http://www.gwtproject.org/)  <!-- in-browser web interface compiler for java -->
* [yew](https://yew.rs/) <!-- in-browser web interface library for rust -->
* Pyodide [@developers2021pyodide] <!-- web assembly python interpreter -->
* Shiny [@chang2020shiny] <!-- web visualizations that interface with R code running on a server -->


## Projects Using the Software

* [AAGOS](https://lalejini.com/Aagos/web/Aagos.html) [@gillespie2018changing]: model to test impact of environmental change on genetic architecture evolution.
* [Conduit](https://uit.readthedocs.io) [@moreno2022best]: library for best-effort communication in high-performance computing.
* [DISHTINY](https://mmore500.com/dishtiny) [@moreno2019toward]: agent-based model to study major transitions in evolution.
* [ecology in evolutionary computation explorer](https://emilydolson.github.io/ecology_of_evolutionary_computation/web/interaction_networks.html) [@dolson2018evological]: interactive visualization of ecological interaction networks in evolutionary computation.
* [Symbulation](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html) [@vostinar2017suicide]: agent-based model for evolution of parasitism, mutualism, and commensalism.
* [SignalGP](https://github.com/amlalejini/signalgp) [@Lalejini2018-GECCO; @moreno2021signalgp]: an event-driven genetic programming substrate.
* [PhylotrackPy](https://github.com/emilydolson/phylotrackpy) [@dolson2024phylotrackpy]: a phylogeny-tracking tool for agent-based evolution, closely integrated with Empirical codebase.
* [Model of cancer evolution on an oxygen gradient](http://emilydolson.github.io/memic_model/web/memic_model.html).

# Acknowledgements

This research was supported in part by NSF grants DEB-1655715 and DBI-0939454, by the National Science Foundation Graduate Research Fellowship under Grant No. DGE-1424871, by Michigan State University through the computational resources provided by the Institute for Cyber-Enabled Research, and by the Eric and Wendy Schmidt AI in Science Postdoctoral Fellowship, a Schmidt Futures program.
Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

# References
