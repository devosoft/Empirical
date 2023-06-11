# Empirical Documentation Documentation

This is a quick primer on how to document things within Empirical.

Empirical makes use of the Sphinx documentation system based off of XML
information gathered from Doxygen via a plugin named Breathe. This means
that Doxygen will automatically build documentation for anything written
in a C++ source file and Sphinx will be used to organize how that
documentation is displayed.

## How to Comment for Doxygen Autodoc

Doxygen has an entire [documentation
section](https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html) on
how to comment your code. We'll provide a trimmed version here so that
you can get started quickly.

Doxygen will examine all comments to determine if they are documentation
comments or just code comments. To make a documentation comment you must
add either an extra \* or /, depending on the kind of comment:

```cpp
/**
 * This is a documentation comment
 * across several lines
 *
 * This comment will be associated with the function immediately following.
*/
void somefunc(sometype param)
{

}

// this is a comment that doxygen will ignore
// note how it only has two leading slashes, like a normal comment
/// This is a comment that will be included in the documentation
/// Note the extra leading slash
/// Huzzah, documentation
```

````{note}
Doxygen requires a minimum of *three* triple slash'd
lines before a block is considered documentation:

```cpp
/// This line will be ignored
int somefunc() { return 5; }

///
/// This line will be included
///
void otherfunc() { ; }
```
````
If you wish to make a more visible comment block, e.g. a header for a
class, then you may do something like the following:

```cpp
/********************************************//**
* Here is some text inside a visible block
***********************************************/
```

```{note}
Doxygen will view this as any other documentation comment and
will not render it any differently than a 'normal' documentation
comment--it is simply more visible within the source code.
```

## How to include Doxygen's autodocs within Sphinx files

Through the use of the Breathe extension it is incredibly easy to
include Doxygen autodocs within a Sphinx documentation file.

Suppose we have a C++ implementation file name `potato.h` that has
inline comment documentation as detailed above and that `potato.h` is a
component of a module named `ingredients` that was just created.

To document them, you must create a file within the Empirical Library
documentation source to hold the module's documentation:

```cpp
    touch doc/library/ingredients.md
```

Within `ingredients.md` you can make an introduction to the module,
etc., and then add in the sphinx directives to include
autodocumentation. Your `ingredients.md` file should look something
like the following:

```md
    # This is the ingredients documentation!

    This is a very short introduction.

    ## potato.h

    ```{eval-rst}
    .. doxygenfile:: potato.h
       :project: Empirical
    ```
```

When the docs are built Sphinx will automatically pull the available
documentation from Doxygen's XML files to construct the docs.

Additional directives exist to include autodocumentaiton from different
levels, the full breakdown of which is available within the [Breathe
Documentation](https://breathe.readthedocs.org/en/latest/directives.html).

## How to add docs to the Sphinx documentation

Sphinx is the system used to generate the developer guide and similar
reference documentation. A primer to using ReStructured Text, the markup
language used by Sphinx, can be found
[here](http://docutils.sourceforge.net/docs/user/rst/quickstart.html).
You can also look at any of the [.rst]{.title-ref} files in the
[doc/]{.title-ref} directory to get a feel for how thinks work.

New documents must be included in the `toctree` in the `index.md`
file for the directory the added file lives in. For example, if you add
`CowFacts.md` to the `CoolFacts/` directory you must add `CowFacts.md`
to the toctree found in `CoolFacts/CowFacts.md`:

```md
    # Cool Facts

    A bunch of cool facts!

    ```{toctree}
    AnteaterFacts
    BirdFacts
    CowFacts
    ```
```

To build the documentation, we recommend you use our [Docker container](https://hub.docker.com/r/devosoft/empirical)
that already has all dependencies
installed. If you would build them on bare metal, you will first need to
have a Python 3 enviroment set up, and install doxygen via your package manager.
After that, simply

```shell
cd doc
pip install -r requirements.txt
```

to install all Python dependencies.
