Empirical Documentation Documentation
=====================================

This is a quick primer on how to document things within Empirical. 

Empirical makes use of the Sphinx documentation system based off of XML information gathered from
Doxygen via a plugin named Breathe. This means that Doxygen will automatically build documentation
for anything written in a C++ source file and Sphinx will be used to organize how that
documentation is displayed.

This primer will be broken up into two sections: 1) how to comment your code so that Doxygen can
automatically pull it out and 2) how to structure the rst files in the doc/ directory so that
Sphinx can construct the docs.

How to Comment for Doxygen Autodoc
----------------------------------

Doxygen has an entire `documentation section
<https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html>`__ on how to comment your code.
We'll provide a trimmed version here so that you can get started quickly.

Doxygen will examine all comments to determine if they are documentation comments or just code
comments. To make a documentation comment you must add either an extra * or /, depending on the
kind of comment::

                /** This is a documentation comment
                across several lines

                This comment will be associated with the function immediately following.
                */
                void somefunc(sometype param)
                {
                
                }

                // this is a comment that doxygen will ignore
                // note how it only has two leading slashes, like a normal comment
                /// This is a comment that will be included in the documentation
                /// Note the extra leading slash

If you wish to make a more visible comment block, e.g. a header for a class, then you may do
something like the following::

                 /********************************************//**
                 * Here is some text inside a visible block
                 ***********************************************/

Note that Doxygen will view this as any other documentation comment and will not render it any
differently than a 'normal' documentation comment--it is simply more visible within the source
code.


How to include Doxygen's autodocs within Sphinx files
-----------------------------------------------------

Through the use of the Breathe extension it is incredibly easy to include Doxygen autodocs within
a Sphinx documentation file.

Suppose we have a c++ implementation file name ``potato.h`` that has inline comment documentation
as detailed above and that ``potato.h`` is a component of a module named ``ingredients`` that was just
created, and you wish to document them.

To do this you must create a file within the Empirical Library documentation source to hold the
module's documentation::

                touch doc/library/ingredients.rst

Within ``ingredients.rst`` you can make an introduction to the module, etc., and then add in the
sphinx directives to include autodocumentation. Your ``ingredients.rst`` file should look
something like the following::

                This is the ingredients documentation!
                ======================================

                This is a very short introduction.

                **potato.h**

                .. doxygenfile:: potato.h
                   :project: Empirical

When the docs are built Sphinx will automatically pull the available documentation from Doxygen's
XML files to construct the docs.

Additional directives exist to include autodocumentaiton from different levels, the full breakdown
of which is available within the `Breathe Documentation
<https://breathe.readthedocs.org/en/latest/directives.html>`__.
