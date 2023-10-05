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

[Scraps document here](https://hackmd.io/@DSAbJHksRtqYhjQ4NWzmGA/S1z3oQDWv)
<!--  @AML (2021-06-12): Doing a comment clean up, moving important ones (e.g., old meeting notes) into the scraps document. -->

<!-- INSTRUCTIONS

- The paper should be between 250-1000 words.

Paper should include:

- A summary describing the high-level functionality and purpose of the software for a diverse, non-specialist audience.
- A clear Statement of Need that illustrates the research purpose of the software.
- A list of key references, including to other software addressing related needs.
- Mention (if applicable) a representative set of past or ongoing research projects using the software and recent scholarly publications enabled by it.
- Acknowledgement of any financial support.

-->

<!-- RESOURCES

- Empirical coding guidelines: https://github.com/devosoft/Empirical/wiki/Coding-Guidelines
- Empirical README: https://github.com/devosoft/Empirical/blob/master/README.md

-->

# Summary
<!-- SUMMARY

JOSS: "Begin your paper with a summary of the high-level functionality of your software for a non-specialist reader. Avoid jargon in this section."

i.e., What does this software do?
-->

# Summary

Empirical is a C++ library designed to promote open science and facilitate the development of scientific software that is efficient, reliable, and easily distributable to researchers and non-experts alike.
Specifically, the library sets out to fulfill the following goals:

1. **Utility:** Empirical features a wide selection of helper tools to streamline common scientific computing tasks such as configuration, data management, random number generation, and mathematical manipulations. Documentation, examples, and project templates streamline developer onboarding.
2. **Efficiency:** Empirical tools emphasize efficiency to make heavy scientific computing workloads tractable. Where possible, computation is moved to compile-time, heap allocation is avoided, and memory-locality is respected.
3. **Reliability:** Scientific simulation and data analysis is meaningful insofar as it is correct. In conjunction with extensive unit testing, Empirical benefits from --- and provides --- sophisticated debug-mode instrumentation tools. These include audited memory management, drop-in replacements for standard C++ containers with safety checking, and self-documenting assertions.
4. **Distributability:** Scientific software should be easily used by any researcher, student, or citizen scientist who wants to experiment, explore, or replicate studies. As such, Empirical is highly portable, uses common data formats, facilitates flexible runtime configuration, and simplifies compilation to a performant web app.

# Statement of Need
<!-- STATEMENT OF NEED
JOSS: illustrates the research purpose of the software.
JOSS reviewer checklist: "clearly state what problems the software is designed to solve and who the target audience is"
-->

Modern web-based interfaces give computational research the unique potential to embody open science objectives of making the scientific process more transparent with auditable and exensible code, clear and replicable methodologies, and production of accessible results [CITE osf.io].
<!-- [@world_academy_of_science_engineering_and_technology_2019] --> 
In practice, however, many scientific software applications are difficult to obtain, install, or use, and produce data in propriatary formats.

High quality open-science tools encourage researchers to follow effective software development practices by simplifying development and helping them improve code quality, scientific rigor, and ease of replication or extension, resulting in broader confidence in and adoption of their software.
In the process, researchers get more out of their own code.
For example, an effective toolkit would allow a developer to easily add a GUI that would not only help other users of the software, but could also help the researcher themsleves gain "soft knowledge" of their system [@10.1145/3185517].

Recent developments in web technology such as WebAssembly enable compilation of native source code to browser-based interactive interfaces.
However, many scientists lack web development training and do not have the bandwidth to learn new languages and frameworks.
Empirical catalyzes progress toward open science ideals by streamlining the development of in-browser software in C++.

To be reuseable and extendable, software must remain robust at scales and in contexts that were not originally envisioned.
Empirical seeks to make writing correct, efficient scientific software easier.
Empirical's debugging suite helps protect against common C++ programming pitfalls such as iterator invalidation, memory leakage, and out-of-bounds indexing.
Bundled algorithms and data structures provide optimized, well-tested drop-in implementations for common scientific computing tasks.
Throughout, library design obviates trade-offs between performance and safety; compile-time switches toggle safety checks for undefined or incorrect behavior.

<!-- However, many scientists do not know how to create browser-based GUIs and lack the resources or incentives to adequately invest in learning.
Empirical addresses these challenges by providing tools for implementing browser-based GUIs using intuitive C++, and without requiring a substantial time investment.
 -->
<!-- @CAO: The below is a critical point, but should probably going in the Availability section below OR in a future paper; I don't think it goes in statement of need. -->
<!-- Even with GUI domain knowledge, maintaining a GUI-based, public-facing code base separately from a command-line, research code base is a burden that often leads to the GUI version lagging behind the research version.
In this circumstance, the research version of the software is no longer truly available to the world.
To avoid this pitfall, Empirical privides tools to implement GUI applications by _wrapping_ (instead of replicating) the research version of the software, making it easier for researchers to keep the most recent version of scientific software available widely.
 -->
<!-- To conduct computationally expensive experiments, researchers often need to run scientific software without incurring overhead associated with GUI integration.In addition, the overhead associated with GUIs (particularly cross-platform or in-browser execution of core code components) must be minimized to be able to realize the full capabilities of scientific software. -->
<!-- Empirical fulfills these requirements by:
* allowing software to be compiled with or without the GUI,
* building the GUI off of highly-efficient Web Assembly, and
* computing everything possible at compile time to increase execution efficiency.
 -->
<!-- Even with an easily available web-based interface, scientific software cannot fulfill open science objectives if its credbility is compromised by undiscovered errors.
Determining the presence of subtle runtime errors in native executables generated using C++ can be difficult, and debugging compiled WASM output from Emscripten can be nearly impossible.
Tools to support researchers in detecting and correcting such errors are critical to open science objectives. -->
<!-- To improve researchers' ability to debug their software, Empirical provides a number of improved debugging functionalities, including wrappers around standard library containers that include extra safety checks, smart pointers in debug mode that convert to raw pointers in release mode, and improved assertions.
These safeguards can be fully disabled by a compiler flag in order to ensure maximum efficiency in performance-critical contexts.
 -->

# Empirical Features

## Facilitating Better Code for Scientific Software

Software produced by academics, especially for one-off use often foregoes rigorous programming practices for the sake of expediency.
By furnishing prepackaged components that address common tasks for scientific software, the Empirical library helps scientific developers efficiently write high quality C++ code.
Utilities for common tasks empower users to craft more readable and maintainable code.
Bugs can be avoided by replacing one-off implementations with Empirical components subjected to structured code review, unit testing (with coverage tracking), and other modern software development practices detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html).
Sustained effort invested into optimization of the library's utilities enables developers to produce efficient software at far less effort.
Finally, off-the-shelf solutions reduce barriers to computational research, especially for developers new to scientific software design patterns.

To these ends, Empirical provides a comprehensive framework to manage runtime configuration and flexible tools for data aggregation and recording.
For example, Empirical's configuration framework includes utilities to

* define and document default configuration values in a single line,
* set configuration values via a combination of a configuration file or command line flags,
* save configuration values to a file,
* perform on-the-fly configuration adjustments, and
* support multiple independent configuration subsystems.

Where appropriate, Empirical's scientific software tools include features that integrate directly into the browser environment.
The configuration framework, for example, accepts input via URL query parameters and ties in with a pre-built, in-browser GUI for setting-by-setting adjustments.

In addition to the core C++ Library, we maintain a [template project](https://github.com/devosoft/cookiecutter-empirical-project) that streamlines laying out crosscompilation boilerplate via a command-line wizard.

High-quality software cannot succeed without a thriving community of engineers.
As detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html), our development practices encourage the meaningful inclusion of all interested contributors, and emphasize the importance of diverse backgrounds and perspectives in the community.

## Realizing the Promise of Emscripten-based Web UIs

Educational or outreach versions of scientific software can provide accessible windows into contemporary scientific work, promoting classroom learning, social media engagement, and citizen science.
Scientific software projects that want to reach these additional audiences must typically maintain multiple codebases.
As the codebases diverge, it becomes time consuming to synchronize changes across them, a problem that is only compounded when also maintiaining crossplatform interfaces.
These development costs preclude many scientific projects from providing easy access to the public.
Even in better-resourced projects, this splintering effect absorbs limited developer hours and often leads to some versions of the code falling into neglect and drifting out of sync.

The Emscripten compiler promises to remedy this source splintering by enabling a single codebase to target web browsers alongside traditional native runtime environments [@zakai2011emscripten].
Browser-based delivery can yield particularly effective public-facing apps due to widespread cross-platform compatibility, no-install access, and rich graphical interfaces.
Native compilations are still required by most scientific applications, however, due to greater speeds and compatibility with high-performance computing environments.

Empirical amplifies the potential of Emscripten by fleshing out its rudimentary interface for interaction with browser elements.
At the lowest level, Empirical provides tools for reciprocal data transfer between C++ code and the browser.
DOM elements (such as `<button>`, `<div>`, and `<canvas>`) are given corresponding C++ objects (`emp::Button`, `emp::Div`, and `emp::Canvas`) and can be easily used from within C++ code.
With these tools, users no longer need to manage JavaScript resources, and thus need much less preexisting web-programing knowledge.
<!-- This facility relieves users of bookkeeping for JavaScript resources (particularly useful for those without web programming domain expertise). -->
At a higher level of abstraction, Empirical packages pre-configured, pre-styled collections of DOM elements as prefabricated widgets (e.g., configuration managers, collapsible read-outs, modal messages, etc.).
Empirical's tools aim to make generating a mobile-friendly, web-based GUI for existing software so trivial that the practice becomes ubiquitous.
<!-- In particular, we are focused on lowering the barrier to entry for developers without domain knowledge in HTML, CSS, and JavaScript by abstracting these matters away behind a C++ interface. -->

Below, we give an example of Empirical's DOM interface in action.
This example creates a button that increments an on-screen counter every time the button is clicked.
You can view the resulting web page live at <https://devosoft.github.io/empirical-joss-demo/>.

C++ source:
```c++
#include "emp/web/web.hpp"

emp::web::Document doc("target");

int x = 5;
int main() {
  doc << "<h1>Hello World!</h1>";
  doc << "Original x = " << x << ".<br>";
  doc << "Current x = " << emp::web::Live(x) << ".<br>";

  // Create a button to modify x.
  emp::web::Button my_button( [](){ x+=5; doc.Redraw(); }, "Click me!" );
  doc << my_button;
}
```

HTML source:
```html
<body>
  <div id="target"> </div>
</body>

<script src="https://code.jquery.com/jquery-1.11.2.min.js" integrity="sha256-Ls0pXSlb7AYs7evhd+VLnWsZ/AqEHcXBeMZUycz/CcA=" crossorigin="anonymous"></script>
<script type="text/javascript" src="main.js"></script>
```

<!-- @MAM: add a code snippet with a brief demo and a screenshot of  the resulting webpage -->

A live demo of more sophisticated Empirical widgets, presented alongside their source C++ code, is available on our [prefab demos page](https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo).

<!-- # Empirical Development Practices -->
<!-- @mmore500 moved to documentation -->
<!-- @AML: talk here about testing/coverage setup, cookiecutter template, etc? maybe cookiecutter could go in re-invent wheel section? -->


## Facilitating Runtime Efficiency

WebAssembly's runtime efficiency is a major driver of its increasing popularity for web app development.
WebAssembly is a virtual bytecode, but just-in-time compilation engines translate critical sections into native machine code [@haas2017bringing].
This compilation model allows WebAssembly to achieve at least 50% --- and at times closer to 90% --- of native performance, providing an order of magnitude speed increase over JavaScript alone [@jangda2019not].
These performance improvements open the door to entirely new possibilities for browser-based scientific computation.
For example, the Avida-ED web viewer at <https://avida-ed.msu.edu/>, uses WebAssembly to simulate hundreds of thousands of generations of digital organisms within the span of a class period.
Such rich, intensive in-browser experiences necessitate efficient source code.

More broadly, Empirical caters to the necessity for runtime efficiency across all scientific computing, including in-browser applications, local runs, or high-performance computing (HPC) cluster deployments.
In some contexts, even modest performance gains can save substantial hardware costs, meaningfully reduce energy use, and shave days or even weeks off run times.
Order-of-magnitude performance gains can meaningfully broaden the scope of scientific questions that are tractable.

Empirical supports runtime efficiency in scientific computing by providing optimized tools for performance-critical tasks.
For example, `emp::BitSet` and `emp::BitVector` classes are faster drop in replacements for their standard library equivilents (`std::bitset` and `std::vector<bool>`) while providing extensive additional functionality for rapid bit manipulations.
Likewise,`emp::Random` wraps a cutting-edge high-performance pseudorandom number generator algorithm [@widynski2020squares].
Benchmark-informed development practices ensure that optimizations translate into consistent performance enhancements.
At a more fundamental level, Empirical's header-only design prioritizes ease of use and runtime performance at the cost of somewhat longer compilation times.

## Facilitating Debugging
<!-- @MAM: help you make your own code reliable -->

Identifying and correcting incorrect program behavior consumes a large fraction of developer hours for any software project.
Software bugs that slip through into production can inflict even greater costs, especially in scientific contexts where the validity of generated data and analyses is paramount.

In conjunction with unit tests and integration tests, runtime safety checks are commonly used to flag potential bugs.
Assert statements typify runtime safety checks.
These statements abort program execution at the point of failure with a helpful error message if an expected runtime condition is not met.
<!-- Users can write `assert` statements into their own code to ensure that program behavior matches expectations.
 -->
Runtime safety checks like `assert` don't necessarily oblige a performance cost to compute the asserted runtime condition; these checks can be verified only in debug mode and ignored in production mode to maximize performance.

Indeed, the C++ standard library's `assert` macro follows this paradigm.
Empirical provides an extended `emp_assert` macro that prints custom error messages with current values of specified expressions, and dispatches a UI alert when triggered in a web environment.
<!-- These features help compensate for the limited tooling currently available in the Empirical web runtime. -->

In addition to user-defined asserts, most programming languages (Java, Python, Ruby, Rust, etc.) provide built-in support to detect common runtime violations, such as out-of-bounds indexing or bad type conversions.
<!-- These built-in protections against runtime violations are considered so critical that many programming languages do not provide a mechanism to disable them for speedups in production code. -->
C++ does not in an effort to maximize performance.
 <!-- provide any standard mechanisms for safety-checking library features. -->
However, standard library vendors  --- like [GCC's `libstdc++`](https://web.archive.org/web/20210118212109/https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html), [Clang's `libc++`](https://web.archive.org/web/20210414014331/https://libcxx.llvm.org/docs/DesignDocs/DebugMode.html), and [Microsoft's `stl`](https://web.archive.org/web/20210121201948/https://docs.microsoft.com/en-us/cpp/standard-library/checked-iterators?view=msvc-160) --- do provide some proprietary support for such safety checks.
This support, however, is limited and poorly documented[^1].
Empirical supplements vendors' runtime safety checking by providing drop-in replacements for `std::array`, `std::optional`, and `std::vector` with stronger runtime safety checks, but only while in debug mode.
In addition, Empirical furnishes a safety-checked pointer wrapper, `emp::Ptr`, that identifies memory leaks and invalid memory access in debug mode while retaining the full speed of raw pointers in release mode.

[^1]: For example, neither GCC 10.3 nor Clang 12.0.0 detect `std::vector` iterator invalidation when appending to a `std::vector` happens to fall within existing allocated buffer space ([GCC live example](https://perma.cc/6WDU-3C8X); [Clang live example](https://perma.cc/6SU9-CUKY)).

<!-- todo add more explanation of emp::Ptr and its rationale -->

Because of poor support for built-in runtime safety checks, C++ developers typically use an external toolchain to detect and diagnose runtime violations.
Popular tools include Valgrind, GDB, and runtime sanitizers.
<!-- (Perhaps, to some degree, this rich toolchain ecosystem enables the ongoing lack of support for such checks within the standard language.) -->
Although this tooling is very mature and quite powerful, there are fundamental limitations to the runtime violations it can detect.
For example, Clang 12.0.0's sanitizers cannot detect the iterator invalidation described above ([live example](https://godbolt.org/z/z6ocqn87W)).
Additionally, most of this tooling is not available when debugging WASM code compiled with Emscripten --- a core use case targeted by the Empirical library.
Although Emscripten provides some [sanitizer support](https://web.archive.org/web/20210513071104/https://emscripten.org/docs/debugging/Sanitizers.html) and [other debugging features](https://web.archive.org/web/20210513070806/https://emscripten.org/docs/porting/Debugging.html), tooling limitations (such as the lack of a steppable debugger) make runtime safety checking particularly critical.

# Outlook and Future Plans

Empirical remains under active development.
Current priorities include assembling compound, higher-level web widgets for common tasks and adding more step-by-step tutorials to our documentation.

However, we are committed to maintaining a stable interface for existing users.
Last year, we took a major step towards this fulfilling this objective on an ongoing basis by completing a major reorganization informed by best practices to expose sustainable, consistent API to our end-users.
We maintain an extensive suite of unit tests and integration tests to ensure that continuing development retains full backwards compatibility.
In addition, our software releases are archived on Zenodo in order to guarantee uninterrupted, perpetual access to our software for those who depend on it [@empirical_2020].

Empirical has already been successfully incorporated into major projects within our research group's primary domains: digital evolution, artificial life, and genetic programming.
We aim for potential utility across a much broader swath of the scientific software ecosystem, however --- particularly among projects that prioritize open science objectives.
To this end, we look forward to supporting a wider of collection of collaborators and end-users.

# Related Software Packages

<!-- JOSS:(including to other software addressing related needs. a representative set of past or ongoing research projects using the software and recent scholarly publications enabled by it.) -->

## Software Addressing Related Needs
<!-- JOSS: Do the authors describe how this software compares to other commonly-used packages?-->


There are many existing software platforms that provide functionalities overlapping with Empirical.
However, most are not in C++, and there is value in this functionality being easily available to C++ programmers.
<!-- being easily available to programmers who are most comfortable in C++ -->
<!-- TODO C++ as a high-efficiency language -->
<!-- Therefore, here we focus only on software platforms that support development in C++. -->
See the Non-C++ Comparable Software section for citations to software platforms that provide some of Empirical's functionality in different languages.

### RepastHPC

RepastHPC, accessible at <https://repast.github.io/>, is a C++ modeling framework targeted at large computing clusters and supercomputers[@collier2013parallel; @north2013complex].
A Java-based counterpart, Repast Simphony, provides interactive GUI support.
As such, simultaneous on-cluster and in-browser support requires maintenance of two separate code bases.

### Boost C++ Libraries

Boost C++ Libraries, available at <https://www.boost.org/>, provide an enormous range of software components.
Several of the Boost libraries have been already incorporated into the C++ standard, and we build off of those with Empirical.
However, Boost does not contain libraries for web-based GUI tools, configuration management, or data management specifically tailored to scientific software.

### Emscripten

Emscripten is available at <https://emscripten.org/> [@zakai2011emscripten].
It provides cross-compilation from C++ to WebAssembly and we use it in Empirical.
Empirical's tools build abstractions from Emscripten intrinsics tailored to visualization and interactive control of scientific simulations.


### Cheerp

Cheerp, another C++ to WebAssembly compiler, is available at <https://leaningtech.com/cheerp/>.
Like Emscripten, Cheerp provides primariliy low-level APIs for interaction with browser GUI elements.

### Non-C++ Comparable Software

<!-- Not going to include discussion, just citations -->

<!--
these are more agent-based simulations which aren't the focus of this paper
* FlameGPU [@richmond2010high]
* NetLogo [@tisue2004netlogo]
* Cell Collective [@helikar2012cell]
 -->

* [TinyGo](https://tinygo.org/) <!-- in-browser web interface compiler for go -->
* [WebIO](https://juliagizmos.github.io/WebIO.jl/latest/) <!-- in-browser web interface library for julia -->
* [GWT](http://www.gwtproject.org/)  <!-- in-browser web interface compiler for java -->
* [yew](https://yew.rs/) <!-- in-browser web interface library for rust -->
* Pyodide [@developers2021pyodide] <!-- web assembly python interpreter -->
* Shiny [@chang2020shiny] <!-- web visualizations that interface with R code running on a server -->


## Projects Using the Software

* [Aagos](https://lalejini.com/Aagos/web/Aagos.html) [@gillespie2018changing]
    * An interactive model for empirically studying how evinronmental change affects the evolution of genetic architectures.
* [conduit](https://conduit.fyi) [@moreno2020conduit]
  * A C++ library that wraps intra-thread, inter-thread, and inter-process communication in a uniform, modular, object-oriented interface, with a focus on asynchronous high-performance computing applications.
* [Dishtiny](https://mmore500.com/dishtiny) [@moreno2019toward]
  * An interactive web demo of an artificial life platform built to study major transitions in evolution.
* [ecology in evolutionary computation explorer](https://emilydolson.github.io/ecology_of_evolutionary_computation/web/interaction_networks.html) [@dolson2018evological]
  * Interactive exploration of interaction networks in evolutionary computation under different selection schemes.
* [Symbulation](https://anyaevostinar.github.io/SymbulationEmp/web/symbulation.html) [@vostinar2017suicide]
  * An interactive artificial life model focused on the evolution of symbiosis between parasitism to mutualism.
    Also features a game to recreate major results from published work.
* [SignalGP](https://github.com/amlalejini/signalgp) [@Lalejini2018-GECCO]
  * SignalGP is a new GP technique designed to give evolution direct access to the event-driven programming paradigm where computations are triggered response to signals from the environment, from other agents, or that are internally generated.
* [Model of cancer evolution on an oxygen gradient](http://emilydolson.github.io/memic_model/web/memic_model.html)
  * A companion model to a series of wet lab experiments on cancer evolution in spatially heterogenous environments

<!-- # Packages used by Empirical -->

<!--Citations to entries in paper.bib should be in
[rMarkdown](http://rmarkdown.rstudio.com/authoring_bibliographies_and_citations.html)
format.

If you want to cite a software repository URL (e.g. something on GitHub without a preferred
citation) then you can do it with the example BibTeX entry below for @fidgit.

For a quick reference, the following citation commands can be used:
- `@author:2001`  ->  "Author et al. (2001)"
- `[@author:2001]` -> "(Author et al., 2001)"
- `[@author1:2001; @author2:2001]` -> "(Author1 et al., 2001; Author2 et al., 2002)"

# Figures

Figures can be included like this:
![Caption for example figure.\label{fig:example}](figure.png)
and referenced from text using \autoref{fig:example}.

Fenced code blocks are rendered with syntax highlighting:
```python
for n in range(10):
    yield f(n)
```
-->
# Acknowledgements

This research was supported in part by NSF grants DEB-1655715 and DBI-0939454 as well as by Michigan State University through the computational resources provided by the Institute for Cyber-Enabled Research.
This material is based upon work supported by the National Science Foundation Graduate Research Fellowship under Grant No. DGE-1424871.
Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

# References
