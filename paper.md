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
    affiliation: 7
  - name: Charles Ofria
    orcid: 0000-0003-2924-1732
    affiliation: "1, 2, 3"
  - name: Emily Dolson
    orcid: 0000-0001-8616-4898
    affiliation: "1, 2, 3"
  - name: Matthew Andres Moreno
    orcid: 0000-0003-4726-4479
    affiliation: "4, 5"
affiliations:
 - name: BEACON Center for the Study of Evolution in Action
   index: 1
 - name: Computer Science and Engineering, Michigan State University
   index: 2
 - name: Ecology, Evolutionary Biology, and Behavior, Michigan State University
   index: 3
 - name: Computer Science, Carleton College
   index: 4
 - name: Ecology and Evolutionary Biology, University of Michigan
   index: 5
 - name: Center for the Study of Complex Systems, University of Michigan
   index: 6
 - name: Computer Science, Grand Valley State University
   index: 7
date: 30 July 2021
bibliography: paper.bib
breaks: false
---

# Summary

Empirical is a C++ library designed to promote open science and facilitate the development of scientific software that is efficient, reliable, and easily distributable to researchers and non-experts alike.
Specifically, the library sets out to fulfill the following goals:

1. **Utility:** Empirical tools streamline common scientific computing tasks such as configuration, end-to-end data management, and mathematical manipulations.
2. **Efficiency:** Empirical implements general-purpose data structures and algorithms that emphasize computational efficiency to support scientific computing workloads.
3. **Reliability:** Empirical provides sophisticated debug-mode instrumentation including audited memory management and safety checked versions of standard library containers.
4. **Distributability:** Empirical is highly portable, uses common data formats, and facilitates compile-to-web app development with object-oriented bindings for Emscripten/WebAssembly GUI elements, all with the goal of building broadly accessible scientific software.

# Statement of Need

High quality open-science tools encourage researchers to follow effective software development practices by improving code quality, scientific rigor, and ease of replication or extension.
Empirical's debugging suite protects against common C++ programming pitfalls, such as iterator invalidation, memory leakage, and out-of-bounds indexing.
Throughout, library design obviates trade-offs between performance and safety; compile-time switches toggle safety checks for undefined or incorrect behavior.

In practice, many scientific software applications are difficult to obtain, install, or use, and produce data in proprietary formats.
Modern web-based interfaces give computational research the potential to better embody open science objectives by making each step of the scientific process more observable [@woelfle2011open].
Empirical leverages recent developments in web technology to compile C++ source code to browser-based interactive interfaces.

# Empirical Features

## Better Code for Scientific Software

Empirical components are subjected to structured code review, unit testing (with coverage tracking), and other modern software development practices detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html).
Sustained effort invested into optimization of the library's utilities enables developers to produce safe and efficient software at far less effort, especially for new developers.
Additionally we provide a [template project](https://github.com/devosoft/cookiecutter-empirical-project) that streamlines laying out crosscompilation boilerplate.

As an example of Emprical's utility, the library provides a configuration framework that includes utilities to

* create documented configuration parameters with default values in a single line of C++ code,
* adjust parameters via configuration files, command line flags, URL query parameters, or in-browser GUIs,
* perform on-the-fly configuration adjustments, and
* support independent configuration subsystems.

High-quality software cannot succeed without an inclusive community of contributors.
As detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html), our development practices emphasize the importance of diverse backgrounds and perspectives in the community.

## Realizing the Promise of Emscripten-based Web UIs

Educational or outreach versions of scientific software can promote classroom learning and citizen science.
The Emscripten compiler enables an existing native codebase to additionally compile to the web [@zakai2011emscripten].
Browser-based delivery can yield particularly effective public-facing apps due to easy access and compelling interfaces.

Empirical amplifies Emscripten by fleshing out its interface for interaction with browser elements.
DOM elements are bound to corresponding C++ objects (e.g., `emp::Button` manages a `<button>` and `emp::Canvas` manages a `<canvas>`) and are easily manipulated from within C++.
Empirical also packages collections of prefabricated web widgets (e.g., configuration managers or collapsible data displays).
These tools simplify generating a mobile-friendly, web-based GUI for existing software.

A live demo of Empirical widgets, presented alongside their source C++ code, is available [here](https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo).

## Runtime Efficiency

WebAssembly's runtime efficiency has driven its adoption in web development [@haas2017bringing], achieving 50% to 90% of native performance [@jangda2019not] and enabling new possibilities for browser-based scientific computation.
For example, [Avida-ED](https://avida-ed.msu.edu/) simulates billions of digital organisms in a day.

More broadly, Empirical provides optimized tools for performance-critical tasks.
For example, `emp::BitArray` and `emp::BitVector` are faster drop-in replacements for their standard library equivalents (`std::bitset` and `std::vector<bool>`) while providing extensive additional functionality.
More fundamentally, Empirical's header-only design prioritizes ease of use and runtime performance, albeit at the cost of longer compilation times.

## Debugging

Undetected software bugs can damage the scientific validity of generated data and analyses.
However, in an effort to maximize performance, C++ eschews common run time safety checks such as out-of-bounds indexing or memory managment errors.
Standard library vendors --- like [GCC's `libstdc++`](https://web.archive.org/web/20210118212109/https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html), [Clang's `libc++`](https://web.archive.org/web/20210414014331/https://libcxx.llvm.org/docs/DesignDocs/DebugMode.html), and [Microsoft's `stl`](https://web.archive.org/web/20210121201948/https://docs.microsoft.com/en-us/cpp/standard-library/checked-iterators?view=msvc-160) --- do provide some support for such safety checks, which is incomplete and poorly documented[^1].
Empirical supplements vendors' runtime safety checking by providing drop-in replacements for standard library containers and even raw pointers,
identifying memory leaks and invalid memory access in debug mode while retaining the full speed of raw pointers in release mode.

Because of poor support for built-in runtime safety checks, C++ developers typically use an external toolchain to detect and diagnose runtime violations.
Popular tools include Valgrind, GDB, and runtime sanitizers.
Although this tooling is very mature and quite powerful, there are substantial limitations to the runtime violations it can detect[^1].
Additionally, most of this tooling is not available when debugging WASM code compiled with Emscripten --- a core use case targeted by the Empirical library.
Although Emscripten provides some [sanitizer support](https://web.archive.org/web/20210513071104/https://emscripten.org/docs/debugging/Sanitizers.html) and [other debugging features](https://web.archive.org/web/20210513070806/https://emscripten.org/docs/porting/Debugging.html), tooling limitations (such as the lack of a steppable debugger) make runtime safety checking provided by Empirical particularly useful.

[^1]: For example, neither GCC 10.3 nor Clang 12.0.0 detect `std::vector` iterator invalidation when appending to a `std::vector` happens to fall within existing allocated buffer space ([GCC live example](https://perma.cc/6WDU-3C8X); [Clang live example](https://perma.cc/6SU9-CUKY)).
Clang 12.0.0's sanitizers also fail todetect this iterator invalidation ([live example](https://perma.cc/4ECQ-D5LG)).

# Outlook and Future Plans

Empirical remains under active development.
Current priorities include assembling higher-level web widgets for common tasks, making existing classes more web-friendly (such as file management and rich text handling), and adding more step-by-step tutorials to our documentation.

We are committed to maintaining a stable interface for existing users.
Our software releases are archived on Zenodo to guarantee access to our software for those who depend on it [@empirical_2020].

Empirical has already been successfully incorporated into major projects within our research group's primary domains: digital evolution, artificial life, and genetic programming.
We aim for potential utility across a much broader swath of the scientific software ecosystem, particularly among projects that prioritize open science objectives.
To this end, we look forward to welcoming new collaborations and supporting a wider collection of end-users.

# Related Software Packages

## Software Addressing Related Needs

Many existing software platforms that provide functionalities overlapping with Empirical.
However, most are not in C++, and there is value in this functionality being easily available to C++ programmers.

### RepastHPC

RepastHPC, accessible at <https://repast.github.io/>, is a C++ modeling framework targeted at large computing clusters and supercomputers[@collier2013parallel; @north2013complex].
A Java-based counterpart, Repast Simphony, provides interactive GUI support.

### Boost C++ Libraries

Boost C++ Libraries, available at <https://www.boost.org/>, provide an enormous range of software components.
However, Boost does not contain libraries for web-based GUI tools, configuration management, or data management specifically tailored to scientific software.

### Emscripten

Emscripten is available at <https://emscripten.org/> [@zakai2011emscripten].
It provides cross-compilation from C++ to WebAssembly and we use it in Empirical.
Empirical's tools build abstractions from Emscripten intrinsics tailored to visualization and interactive control of scientific simulations.

### Cheerp

Cheerp, another C++ to WebAssembly compiler, is available at <https://leaningtech.com/cheerp/>.
Like Emscripten, Cheerp provides primariliy low-level APIs for interaction with browser GUI elements.

### Non-C++ Comparable Software

* [TinyGo](https://tinygo.org/) <!-- in-browser web interface compiler for go -->
* [WebIO](https://juliagizmos.github.io/WebIO.jl/latest/) <!-- in-browser web interface library for julia -->
* [GWT](http://www.gwtproject.org/)  <!-- in-browser web interface compiler for java -->
* [yew](https://yew.rs/) <!-- in-browser web interface library for rust -->
* Pyodide [@developers2021pyodide] <!-- web assembly python interpreter -->
* Shiny [@chang2020shiny] <!-- web visualizations that interface with R code running on a server -->


## Projects Using the Software

* [Aagos](https://lalejini.com/Aagos/web/Aagos.html) [@gillespie2018changing]: model to test impact of environmental change on genetic architecture evolution.
* [conduit](https://conduit.fyi) [@moreno2020conduit]: library for best-effort communication in high-performance computing.
* [Dishtiny](https://mmore500.com/dishtiny) [@moreno2019toward]: agent-based model to studdy major transitions in evolution.
* [ecology in evolutionary computation explorer](https://emilydolson.github.io/ecology_of_evolutionary_computation/web/interaction_networks.html) [@dolson2018evological]: interactive visualization of ecological interaction networks in evolutionary computation.
* [Symbulation](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html) [@vostinar2017suicide]: agent-based model for evolution of parasitism, mutualism, and commensalism.
* [SignalGP](https://github.com/amlalejini/signalgp) [@Lalejini2018-GECCO]: an event-driven genetic programming substrate.
* [Model of cancer evolution on an oxygen gradient](http://emilydolson.github.io/memic_model/web/memic_model.html).

# Acknowledgements

This research was supported in part by NSF grants DEB-1655715 and DBI-0939454, by the National Science Foundation Graduate Research Fellowship under Grant No. DGE-1424871, by Michigan State University through the computational resources provided by the Institute for Cyber-Enabled Research, and by the Eric and Wendy Schmidt AI in Science Postdoctoral Fellowship, a Schmidt Futures program.
Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

# References
