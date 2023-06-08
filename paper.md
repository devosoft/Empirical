---
title: 'Empirical: A support library for scientific research, education, and public engagement'
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

Empirical is a C++ library designed to promote open science and facilitate the development of scientific software that is efficient, reliable, and broadly available.
Specifically, we intend for both the library and the software built with it to meet the following goals:

1. **Utility:** Empirical features a wide selection of helper tools to streamline common scientific computing tasks such as configuration, data management, random number generation, mathematical manipulations, and a signal-action system.  We include examples, documentation , and project templates so that developers can quickly and easily build high quality scientific software.
2. **Efficiency:** Empirical tools emphasize efficiency, with the recognition that scientific software can take hours, days, or even months to run.  Empirical tools are heavily optimized, often offloading computation to compile-time, and easily configured for varying use cases.
3. **Reliability:** Correctness is critical to scientific software.  In addition to extensive testing on the library itself, Empirical provides a range of sophisticated debugging tools, all of which can be deactivated to maintain efficiency.  These include audited memory management, drop-in replacements for standard C++ containers with safety checking, and improved assert capabilities.
4. **Availability:** Scientific software is most useful when it can be easily accessed by any researcher, student, or citizen scientist who wants to experiment, explore, or test the reproducability of results.  As such, Empirical is highly portable, uses common data formats, flexible and intuitive configuration options, and tools to turn your software into a performant web app, without requiring a deep understanding of web development.
   <!-- Additionally, we strive for inclusivity in our own development practices and for our library to reduce barriers to entry for software developers and users.-->

These core philosophies have guided Empirical's inception, design, and development in service of open science objectives.

<!-- Empirical helps scientific software manifest open science objectives by providing tools to wrap existing software with an interactive in-browser web interface without compromising researchers' primary software objectives.
Simultaneously, Empirical supports correctness and performance by providing drop-in debugging tools and efficient, elegant tools for common tasks such as configuration and data collection. -->

<!-- Empirical has already been successfuly incorporated into major projects within our research group's primary domains: digital evolution, artificial life, and evolutionary computing.
We aim for potential utility across a much broader swath of the scientific software ecosystem, however, particularly among projects that prioritize open science objectives.
To this end, we look forward to supporting a wider of collection of end-users and establishing a wider network of collaborations. -->

# Statement of Need
<!-- STATEMENT OF NEED
JOSS: illustrates the research purpose of the software.
JOSS reviewer checklist: "clearly state what problems the software is designed to solve and who the target audience is"
-->


<!-- TODO: Switch some sentences from new->old to old->new for smoothness -->

<!--The philosophy of open science is-->
The Open Science movement aims to put research "in the hands of as many as possible" [@world_academy_of_science_engineering_and_technology_2019].
In practice, however, many scientific software applications are *inaccessible* because they are prohibitively difficult to obtain, install, use or understand.<!-- @CAO: We could also say something about specialized or high-end equipment often required to run it. -->
Scientific software is typically developed by specialists who are incentivised to share their results, but not the tools that they crafted to obtain those results.
Projects that are accessible to only a single author or small group can fundamentally undermine the credibility of scientific software, especially when testing and debugging protocols are unclear.
Conversely, opening up these projects more broadly, will not only improve the reliability and utility of the underlying code, but also increase the quality of the science through broad replication and extension of studies, increased collective understanding, and accelerated rates of discovery.
<!-- Despite these challenges, scientific software has incredible potential to radically embody open science ideals due to the transparency of open-source code and the near-universal capacity (in principle) to reproduce aspects of computational science due to ubiquity of internet-connected consumer-grade hardware, including among laypeople. -->

Many scientists do not know how to make web applications, especially with visualizations, and do not have the bandwidth to learn entirely new languages and frameworks.
Such software improvements, however, help beyond accessibility of the software; for example adding an interactive GUI can help even the researcher who wrote the software, *e.g.,* by visually observing an experiment to gain "soft knowledge" of their system [@10.1145/3185517].
Empirical catalyzes progress toward this open science ideal by streamlining the development of efficient and reliable software that can be made accessible from a web browser using intuitive C++, and without requiring a substantial time investment.
Beyond the improvements to reliability inherent from having more people using a piece of software, Empirical also provides a range of debugging tools to help ensure that common programming pitfalls are avoided.
For efficiency Empricial provides a vast array of optimized tools commonly needed in scientific software, as well as the ability to toggle components that incur a performance cost such a the web GUI or debugging tools.



<!-- However, many scientists do not know how to create browser-based GUIs and lack the resources or incentives to adequately invest in learning.
Empirical addresses these challenges by providing tools for implementing browser-based GUIs using intuitive C++, and without requiring a substantial time investment.
 -->
<!-- @CAO: The below is a critical point, but should probably going in the Availability section below OR in a future paper; I don't think it goes in statement of need. -->
<!-- Even with GUI domain knowledge, maintaining a GUI-based, public-facing code base separately from a command-line, research code base is a burden that often leads to the GUI version lagging behind the research version.
In this circumstance, the research version of the software is no longer truly available to the world.
To avoid this pitfall, Empirical privides tools to implement GUI applications by _wrapping_ (instead of replicating) the research version of the software, making it easier for researchers to keep the most recent version of scientific software available widely.
 -->
<!-- To conduct computationally expensive experiments, researchers often need to run scientific software without incurring overhead associated with GUI integration.
In addition, the overhead associated with GUIs (particularly cross-platform or in-browser execution of core code components) must be minimized to be able to realize the full capabilities of scientific software. -->
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

The core Empirical support library comprises four major components:

1. debug-instrumented fundamental type and STL container wrappers,
2. implementations of general-purpose data structures and algorithms,
3. integrated, end-to-end frameworks for data and configuration management, and
4. object-oriented bindings for Emscripten/WebAssembly GUI elements.

Several additional specialized tools are co-developed within the Empirical repository, notably including the next-generation Modular Agent Based Evolution Framework (MABE 2) and a digital phylogeny-tracking tool suite.
Given their substantial scope, these projects merit separate consideration and fall outside the scope of present discussion of the core support library.

## Facilitating Better Code for Scientific Software

Software produced by academics, especially for one-off use, is often cobbled together, foregoing better programming practices for the sake of expediency.
To some extent, this outcome seems inevitable given the typical conditions endemic to academic software development: one-and-done projects assembled under chafing time restrictions where the developer and consumer are one and the same, and often early-career with core domain expertise outside of software development.
All the while, writing code can constitute a major time sink --- and a major source of frustration --- for academics.

By furnishing prepackaged components that address common tasks for scientific software, the Empirical library helps end users to write better C++ code and to do so faster.
Because the library provides utilities for common tasks that would otherwise require verbose inline implementations, users can better leverage abstraction to craft more readable and maintainable code.
Because the library's utilities are more extensively vetted and tested than
most code for one-off projects, users can increase software reliability by avoiding potential bugs in bespoke implementations.
Additionally, more effort can be invested into optimizing the library's utilities for performance than would be reasonable for one-off implementations, paying off in more efficient software for the end-user.
Finally, the library reduces barriers to entry for scientific software developers by providing off-the-shelf solutions for many of the common tasks new developers struggle through while coming up to speed on development patterns for scientific software.

Empirical's tools specially target common scientific computing tasks.
Empirical provides 1) a comprehensive framework to manage runtime configuration, 2) flexible tools for data aggregation and recording, and 3) prefabricated UI components for the web browser.
For example, Empirical's configuration framework includes utilities to

* define and document default configuration values,
* accept configuration adjustments via the command line,
* accept configuration adjustments via a configuration file,
* dump configuration settings to a file,
* perform on-the-fly configuration adjustments,
* and leverage an object-oriented configuration packaging model to manage coexisting configuration tableaus.

Where appropriate, Empirical's scientific software tools include features that integrate directly into the browser environment.
The configuration framework, for example, accepts input via URL query parameters and ties in with a pre-built, in-browser GUI for setting-by-setting adjustments.

In addition to the core C++ Empirical Library, we maintain a [template project](https://github.com/devosoft/cookiecutter-empirical-project) that streamlines the process of organizing a new project.
The heavy lift of laying out nice boilerplate for projects using Emscripten, in particular, motivated us to put together this template.
Our template leverages the [cookiecutter project](https://github.com/cookiecutter/cookiecutter) to automate template instantiation via a command-line wizard.

Finally, any effort to promote development of quality software cannot fully succeed without a thriving community of engineers.
As detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html), our development practices incorporate intentional action to support diversity, equity, and inclusion.

## Facilitating Runtime Efficiency

WebAssembly's runtime efficiency drives much of the growing interest in using it (instead of traditional JavaScript) to program in the web browser.
WebAssembly binaries compiled with Emscripten manifest significant performance gains over comparable vanilla JavaScript programs by eschewing overheads associated with the JavaScript language (safety checks, automatic garbage collection, runtime polymorphism, etc.).
WebAssembly itself is simply an interpreted bytecode;
under most cases, we would expect a several-fold slowdown compared to comparable machine code.
In some cases, this expected slowdown due to the overhead of interpretation might constitute a full order of magnitude [@kazi2000techniques].
However, browsers' just-in-time compilation engines allow critical sections of the WebAssembly bytecode to be translated into native machine code [@haas2017bringing].
This just-in-time compilation model allows WebAssembly to achieve at least 50% --- and often closer to 90%  --- of native performance [@jangda2019not].

The potential for near-native performance in browser from software compiled with Emscripten opens the door to entirely new possibilities for web-facing scientific computation.
For example, as accomplished by the Avida-ED web viewer at <https://avida-ed.msu.edu/>, simulating hundreds of thousands of generations of genetic program evaluations within the span of a class period.
To successfully deliver such rich, intensive in-browser experiences via WebAssembly, the backing C++ source must also play a role in maximizing efficiency.

Runtime efficiency is equally critical in the primary native use case targeted by Empirical: digital experiments and general scientific computing.
In this context, replicate jobs running the native executable might occupy dozens or hundreds of compute cores around the clock.
A 10 or 20% performance gain can save substantial hardware costs, meaningfully reduce net energy use, and shave days or even weeks off of time-to-result for long-running computations.
Order-of-magnitude performance gains might shift some otherwise intractable experiments into the realm of practicality, broadening the scope of scientific questions that can be meaningfully investigated.

Empirical facilitates runtime efficiency in end-user code by supplying highly-optimized tools for performance-critical tasks.
For example, `emp::BitSet` and `emp::BitVector` classes support manipulation of fixed- and variable-length bitstrings, making extensive use bit-level optimizations to yield order-of-magnitude speedups.
Likewise,`emp::Random` wraps a cutting-edge high-performance pseudorandom number generator algorithm [@widynski2020squares].
These tools were implemented with benchmark-informed development practices, guiding selection to ensure that optimizations successfully translate into practical performance enhancements.
At a more fundamental level, the library's header-only design prioritizes runtime performance by trading longer compilation time for potential performance gains under optimizing compilation (as well as greater flexibility and a simplified build process for the end user).


## Facilitating Debugging
<!-- @MAM: help you make your own code reliable -->

For any software development project, identifying and correcting incorrect program behavior consumes a large fraction of developer hours.
Worse yet, software bugs that slip through into production can inflict even greater costs, especially in the context of scientific software where the validity of generated data and analyses is paramount.

In conjunction with unit tests and integration tests, runtime safety checks are commonly used to track down the root causes of known bugs and guard against undetected bugs.
Assert statements typify runtime safety checks.
These statements abort program execution at the point of failure with a helpful error message if an expected runtime condition is not met.
<!-- Users can write `assert` statements into their own code to ensure that program behavior matches expectations.
 -->
Runtime safety checks like `assert` oblige a performance cost, however.
Computing and testing the asserted runtime condition consumes CPU cycles.
To ameliorate such costs, many programming languages support a distinction between production mode and debug mode.
Assert statements and other runtime safety checks are verified in debug mode and ignored in production mode to maximize performance.

The C++ standard library provides an `assert` macro that includes the provided assertion in debug mode builds and eliminates it from production mode builds.
Empirical provides a comparable `emp_assert` macro, differentiated primarily in terms of built-in support for the web runtime.
This macro dispatches a UI alert when triggered, allows for users to write their own error messages, and enables registration of additional variables to be printed in any error message from that assert statement.
These features help compensate the limited debugging tooling available in the web runtime.

In addition to user-defined asserts, most programming languages provide built-in support to detect common runtime violations,
such as out-of-bounds indexing into a collection or bad type conversions.
These built-in protections against runtime violations are considered so critical that many programming languages --- such as Java, Python, and Ruby --- do not provide a mechanism to disable them for speedups in production code.
C++ takes the opposite tack and does not provide any standard mechanisms for safety-checking library features.
However, standard library vendors  --- like [GCC's `libstdc++`](https://web.archive.org/web/20210118212109/https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html), [Clang's `libc++`](https://web.archive.org/web/20210414014331/https://libcxx.llvm.org/docs/DesignDocs/DebugMode.html), and [Microsoft's `stl`](https://web.archive.org/web/20210121201948/https://docs.microsoft.com/en-us/cpp/standard-library/checked-iterators?view=msvc-160) --- provide some support for such safety checks.
This support, however, is limited and poorly documented.
For example, neither GCC 10.3 nor Clang 12.0.0 detect `std::vector` iterator invalidation when appending to a `std::vector` happens to fall within existing allocated buffer space ([GCC live example](https://perma.cc/6WDU-3C8X); [Clang live example](https://perma.cc/6SU9-CUKY)).
Empirical supplements vendors' runtime safety checking by providing drop-in replacements for `std::array`, `std::optional`, and `std::vector` with stronger runtime safety checks while in debug mode.
In addition, Empirical furnishes a safety-checked pointer wrapper, `emp::Ptr`, that detects memory leaks and invalid memory access in debug mode while retaining the full speed of raw pointers in release mode.

<!-- todo add more explanation of emp::Ptr and its rationale -->

Because of poor support for built-in runtime safety checks, C++ developers typically use an external toolchain to detect and diagnose runtime violations.
Popular tools include Valgrind, GDB, and runtime sanitizers.
<!-- (Perhaps, to some degree, this rich toolchain ecosystem enables the ongoing lack of support for such checks within the standard language.) -->
Although this tooling is very mature and very powerful, there are fundamental limitations to the runtime violations it can detect.
For example, Clang 12.0.0's sanitizers cannot detect the iterator invalidation described above ([live example](https://godbolt.org/z/z6ocqn87W)).
Additionally, most of this tooling is not available when debugging WASM code compiled with Emscripten --- a core use case targeted by the Empirical library.
Although Emscripten provides some [sanitizer support](https://web.archive.org/web/20210513071104/https://emscripten.org/docs/debugging/Sanitizers.html) and [other debugging features](https://web.archive.org/web/20210513070806/https://emscripten.org/docs/porting/Debugging.html), tooling limitations (such as the lack of a steppable debugger) make top-notch runtime safety checking particularly critical in this environment.

As detailed [in our documentation](https://empirical.readthedocs.io/en/latest/dev/empirical-development-practices.html), we use modern development practices such as continuous integration, unit testing, and test coverage analysis in order to ensure that the library itself is reliable.

## Realizing the Promise of Emscripten-based Web UIs

Because of the ubiquitous availability of desktop computers and the ease of distributing software over the internet, scientific software has unique potential for low-barrier, hands-on outreach across broad educational and general audiences.
Public-facing software also enables better science by making interactive demonstrations available to researchers outside a project's core development group.
Additionally, public-facing software exemplifies open science by proffering accessible windows into contemporary scientific work.

However, research versions of software are typically specialized to run on high-performance computing environments, often require extensively involved install processes, and generally lack graphical interfaces.
Effective public-facing software, on the other hand, requires compatibility with consumer-grade hardware, a streamlined install process, and an intuitive graphical user interface.

For these reasons, many scientific software projects have historically maintained separate code bases for research versus education and outreach.
The Avida-ED project, for example, leveraged the existing Avida digital evolution model system for classroom activities [@pennock2007learning], but could not effectively leverage Avida's existing code base.
Additionally, as was the case for the Avida-ED project, significant code duplication is often required to support the same education and outreach product across different operating systems.

This source splintering phenomenon precludes projects with limited resources from offering an education and outreach product.
In better-resourced projects, this splintering effect absorbs limited (and expensive) developer hours and often leads to one version of the code --- usually that for education/outreach --- falling into neglect and drifting out of sync with other versions.

The Emscripten compiler promises to remedy source splintering by enabling software compilation to target web browsers in addition to the traditional native runtime environment [@zakai2011emscripten].
Well-constructed browser-based apps excellently fulfill the requirements for effective public-facing software due to widespread cross-platform compatibility, no-install access, and rich graphical interfaces.
With Emscripten, a single core code base can serve double duty.

Empirical amplifies the potential of Emscripten by fleshing out the rudimentary interface Emscripten exposes for interacting with browser elements.
At the lowest level, Empirical provides tools that facilitate convenient data transfer back and forth between C++ code and the Web Assembly runtime.
On top of this, Empirical provides an object-oriented framework that wraps DOM elements (_e.g.,_ `<button>`, `<div>`, `<canvas>`, etc.) to easily control them and react to their HTML events from within C++ code.
This facility relieves users of bookkeeping for JavaScript resources or even writing any JavaScript by hand, which is particularly useful for those without web programming domain expertise.
At a still higher level of abstraction, Empirical packages pre-configured, pre-styled collections of DOM elements as prefabricated widgets (configuration managers, collapsible read-outs, modal messages, etc.).
Our ultimate aspiration for Empirical is to make implementing a mobile-friendly, web-based GUI for existing software so trivial that the practice becomes ubiquitous.
In particular, we are focused on lowering the barrier to entry for developers without domain knowledge in HTML, CSS, and JavaScript by abstracting these matters away behind a C++ interface.

Below, we give an example of Empirical's DOM interface in action.
This example creates a button that increments an on-screen counter every time the button is clicked.
You can view the resulting web page live at <https://devosoft.github.io/empirical-joss-demo/>.

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


```html
<body>
  <div id="target"> </div>
</body>

<script src="https://code.jquery.com/jquery-1.11.2.min.js" integrity="sha256-Ls0pXSlb7AYs7evhd+VLnWsZ/AqEHcXBeMZUycz/CcA=" crossorigin="anonymous"></script>
<script type="text/javascript" src="main.js"></script>
```

<!-- @MAM: add a code snippet with a brief demo and a screenshot of  the resulting webpage -->

A live demo of more sophisticated Empirical prefabricated widgets, presented alongside their source C++ code, is available on our [prefab demos page](https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo).

<!-- # Empirical Development Practices -->
<!-- @mmore500 moved to documentation -->
<!-- @AML: talk here about testing/coverage setup, cookiecutter template, etc? maybe cookiecutter could go in re-invent wheel section? -->

# Outlook and Future Plans

Empirical remains under active development.
Current priorities include assembling compound, higher-level web widgets for common tasks, developing a comprehensive framework for digital evolution experiments, and adding more step-by-step tutorials to our documentation.

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


There are many existing software platforms that provide support for the different functionalities that Empirical also provides.
However, most are not in C++, and there is value in this functionality being easily available to programers .
<!-- being easily available to programmers who are most comfortable in C++ -->
<!-- TODO C++ as a high-efficiency language -->
Therefore, here we focus only on software platforms that support development in C++.
See the Non-C++ Comparable Software section for citations to software platforms that provide some of Empirical's functionality in a different language.



### RepastHPC

Repast for High Performance Computing is available at <https://repast.github.io/> and is a "C++ based modeling system that is designed for use on large computing clusters and supercomputers" [@collier2013parallel; @north2013complex].
RepastHPC is part of a larger suite that includes Repast Simphony, which is a "Java-based modeling system" that includes support for interactive GUIs.
This suite is a classic example of having two separate versions of software, one for research and one for visualization, that must be separately maintained.

### Boost C++ Libraries

Boost C++ Libraries is available at <https://www.boost.org/>.
Boost libraries provide a wide range of tools that improve the writability and efficiency of C++ code.
Several of the Boost libraries have been already incorporated into the C++ standard, and we build off of those with Empirical.
However, Boost does not contain libraries for web-based GUI tools, configuration management, or data management specifically tailored to scientific software.

### Emscripten

Emscripten is available at <https://emscripten.org/> [@zakai2011emscripten].
It provides cross-compilation from C++ to Javascript and we use it in Empirical.
However, we have added tools that provide more intuitive commands for C++ programmers creating visualizations of scientific software such as simulations.


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
