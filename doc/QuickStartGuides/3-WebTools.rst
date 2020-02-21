Web Tools
=========

The web tools in Empirical are structured to allow the developer to fully control components of
a web page from C++.  Note that you must have the emscripten compiler installed for web utilities
to function properly.

Empirical web Widgets include Text, Buttons, Images, Tables, or many other HTML components.
All widgets are derived from :code:`emp::Widget` and structured such that multiple widgets can
properly refer to and modify the same component.

A Simple Example
----------------

In order to get a simple example working, you need:

- an HTML file (the default version is effective in most cases),
- the C++ code file to actually control the web page, and
- the necessary flags to compile (again, the default version should work).

The required files are all in :code:`examples/web/` called :code:`Example.html`, :code:`Example.cc` and :code:`Makefile`.

Here is :code:`Example.html`:

.. code-block:: html

  <!doctype html>
  <html>
  <head>
  <meta charset="utf-8">
  <title>Example</title>
  </head>
  <body>
  <div id="emp_base"></div>
  <script src="jquery-1.11.2.min.js"></script>
  <script type="text/javascript" src="Example.js"></script>

  </body>
  </html>

The only portions of this file that should be customized for your project are the title (in
the :code:`<title>` tags) and the JavaScript file (change :code:`Example.js` to the name of the :code:`.js` file that
you want to generate).  The div, named :code:`"emp_base"`, is where we will hook Empirical to this web
page.  You MAY add more divs to control or customize any aspects of this file's CSS, but none
of this is required for developing an interface, and most of it can be controlled via C++.

Here is the :code:`Example.cc` file:

.. code-block:: C++

  #include "web/web.h"

  namespace UI = emp::web;

  UI::Document doc("emp_base");

  int main() {
    doc << "<h1>Hello World!</h1>";
  }

This setup will create a web page the says "Hello World!" in large letters.  Manipulations to
doc will allow us to control many portions of the web page.  To break down what's going on,
including :code:`web/web.h` pulls in Empirical's web framework.  Setting the namespace to :code:`UI`
provides a shorter prefix for accessing widgets.  Creating the :code:`doc` :code:`Document` object attaches
to the div by the same :code:`"emp_base"` name in the HTML file.  Note the :code:`doc` is global because we
want the web page to keep functioning after :code:`main()` finishes.  In emscripten, :code:`main()` is like any
other function and does not force the termination of the execution.

Now that we have the two files we need, the only other thing we need to do is compile.  The
provided Makefile can be run by typing :code:`make Example.js`.  This will trigger:

.. code-block:: bash

  emcc -std=c++14 -Wall -Wno-unused-function -I../../source/ -Os
  -s TOTAL_MEMORY=67108864 --js-library ../../source/web/library_emp.js
  -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
  Example.cc -o Example.js

- emscripten uses the :code:`emcc` compiler (or :code:`em++`, since we are using C++).
- :code:`-std=c++14` : Empirical requires c++14.
- :code:`-Wall -Wno-unused-function` : turn on all warnings by default except for unused functions, since not all library functions are going to be used.
- :code:`-I../../source/` : The compiled file is two directories up in the Empirical library, so this flag properly includes the source files.
- :code:`-Os` : Optimize for size, though :code:`-O3` may perform better in some situations.
- :code:`-s TOTAL_MEMORY=67108864` : Make sure we have enough memory; in this case reserve 64 MB.
- :code:`--js-library ../../source/web/library_emp.js` : Load Empirical JS functions that we might need.
- :code:`-s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']"` : Make sure we can run the C++ functions :code:`main()` and :code:`_empCppCallback()` from Javascript, to facilitate two-way communication.
- :code:`-s NO_EXIT_RUNTIME=1` : In most cases, we don't want our program to stop when :code:`main()` finishes.

...finally we list the source file we are compiling (:code:`Example.cc`) and the output file that we
want to produce (:code:`Example.js`).  The majority of these flags shouldn't change from one
compilation to the next, other than changing optimization to debugging options, which we'll
cover in the debugging guide.

If you want to speed things up further, you may also want to add the compiler option

 :code:`-DNDEBUG` : turn off debugging for a faster executable

To test the results, open :code:`Example.html` in your web browser!


Controlling a Web Page: The Basics
----------------------------------

To understand how most of the HTML widgets work we need only change the main code file; the
current HTML file and compiler options can be left the same.

As indicated by our starting point, text can be streamed into an HTML document in a similar
way to an output stream in the standard library.  For example, we can update our main function
to mix text and variables:

.. code-block:: C++

  int main() {
    int x = 5;
    doc << "<h1>Hello World!</h1>";
    doc << "x = " << x << ".<br>";
  }

In additional to regular variables, :code:`emp::Document` (and other Empirical web containers) can also
take a range of Empirical Widgets.

.. code-block:: C++

  void Ping() { doc << "Ping! "; }
  int main() {
    int x = 5;
    doc << "<h1>Hello World!</h1>";
    doc << "x = " << x << ".<br>";

    // Insert an image (in place)
    doc << UI::Image("url.goes.here") << "<br>";

    // Create a button and then insert it.
    UI::Button my_button( Ping, "Click me!" );
    doc << my_button;
  }

Notice now that you not only have a pretty picture, but you also have a button that will add
new text on to the screen each time it's clicked.

But what if we want to update existing content?  We can do this in two ways: either by marking
a variable (or function) as "Live" or by simply changing a widget that is already on the
screen.


"Live" Variables and Function
-----------------------------

Anything sent to a web page that is inside a :code:`UI::Live()` function will always have its most current
value used whenever that portion of the page is redrawn.  For example, let's make our button
modify the value of :code:`x` and redraw it.

.. code-block:: C++

  int x = 5;
  int main() {
    doc << "<h1>Hello World!</h1>";
    doc << "Original x = " << x << ".<br>";
    doc << "Current x = " << UI::Live(x) << ".<br>";

    // Create a button to modify x.
    UI::Button my_button( [](){ x+=5; doc.Redraw(); }, "Click me!" );
    doc << my_button;
  }

Notice that we also moved :code:`x` to be a global variable.  This is because if it were local to :code:`main()`
it would be freed as soon as main ended.  We're also using a lambda this time instead of a
previously defined function.  Either option is fine.

Try clicking on the button -- you'll see that :code:`x` will be updated, and then the document is
signaled that it needs to redraw, so the change is reflected on the screen.  Note that we didn't
actually need to redraw the entire document to update :code:`x`, just the :code:`Text` widget it is in; we'll
talk more about how to do that below.

Of course, we can put a function in the :code:`UI::Live()` and that function will be called each time
the containing Widget is redrawn.

.. code-block:: C++

  int x = 5;
  int main() {
    doc << "<h1>Hello World!</h1>";
    doc << "Original x = " << x << ".<br>";
    doc << "Current x = " << UI::Live(x) << ".<br>";
    doc << "x/5 = " << UI::Live( [](){ return x/5; } ) << ".<br>";

    // Create a button to modify x.
    UI::Button my_button( [](){ x+=5; doc.Redraw(); }, "Click me!" );
    doc << my_button;
  }

Of course, we need to be able to modify Widgets in addition to variables; fortunately this is
easy as well.


Finding and Modifying Existing Widgets
--------------------------------------

There are two ways to keep track of Widgets in Empirical.  One is to simply hold on to a
variable associated with the Widget, such as :code:`my_button` in our previous examples.  At any
point we can still modify something about :code:`my_button`.  For example, if we added a line at the
end of main:

.. code-block:: C++

    my_button.Label("PLEASE Click Me!");

We will see that the button label updates to the new string.

The other option we have to keep track of a widget is to specify its HTML identifier so that
we can look it up again later.  For example, if when we first declared :code:`my_button` we had given
it an extra string argument, that string would be used as its identifier.

.. code-block:: C++

    UI::Button my_button( [](){ x+=5; doc.Redraw(); }, "Click me!", "my_button" );

At any point after we insert the button into a container (such as :code:`doc`), we are able to request
it back from the container again.  So, for example instead of

.. code-block:: C++

    my_button.Label("PLEASE Click Me!");

we could have said

.. code-block:: C++

    doc.Button("my_button").Label("PLEASE Click Me!");

and :code:`doc` will properly look up the correct button for us (or trip an assert if the required
button cannot be found.)  In practice, allowing containers to track Widgets is much easier
than juggling links to all of them yourself.


Controlling CSS
---------------

Web page aesthetics are controlled by adjusting the CSS of the widgets, and Empirical is no
different.  You have two options for controlling CSS -- you can do it the traditional way by
modifying the HTML file (often with the help of other packages) or else you can control
specific CSS settings with the :code:`.CSS` member function associated with all Widgets.

For example, if we wanted our button to be green with red text, we could add to the end of
:code:`main` the statement

.. code-block:: C++

    my_button.SetCSS("background-color", "green").SetCSS("color", "red");

Note the chaining of modifiers.

Most common settings are directly defined as member functions, so the above could also be
expressed as

.. code-block:: C++

    my_button.SetBackground("green").SetColor("red");

which can be cleaner.  See the class documentation for a full list of available functions to
modify each Widget type.


Empirical Tables
----------------

Tables are one of the features of Empirical that differs most in style from the underlying
HTML it modifies, preferring a more exact form where the user sets the number of rows and
columns to be used.

For example, to build a table with 7 rows and 3 columns, we can declare

.. code-block:: C++

    UI::Table my_table(7, 3, "my_table");

To access a cell from a table, we can simply use the :code:`.GetCell(x,y)` member function.  So if
we want to fill the table with data, we might do something like

.. code-block:: C++

    for (size_t r = 0; r < 7; r++) {
      for (size_t c = 0; c < 3; c++) {
      	my_table.GetCell(r,c) << (r+3*c);
      }
    }

To make the table pretty, we probably want to add some CSS.

.. code-block:: CSS

    my_table.SetCSS("border-collapse", "collapse");
    my_table.SetCSS("border", "3px solid");
    my_table.CellsCSS("border", "1px solid");
    my_table.CellsCSS("padding", "3px");

Note that we are able to target the CSS of all :code:`TableCells` at once.

We can target individual cells using :code:`GetCell()`, as well as using :code:`GetRow()`, :code:`GetCol()`,
:code:`GetRowGroup()`, and :code:`GetColGroup()`.  Each of these returns a :code:`TableWidget` with the appropriate
component in focus, so additional modifications are handled correctly.

Finally, of course, make sure to insert the table into the document.

.. code-block:: C++

    doc << my_table;


Canvas
------

Canvas widgets in Empirical are a slightly streamlined version of canvases in HTML.  To build
one, you simply need to create an :code:`emp::Canvas` object (with the appropriate size) and place it
into the document.  For example

.. code-block:: C++

    UI::Canvas my_canvas(300, 400, "my_canvas");

would create a 300x400 canvas.  We can then use member functions to draw lines, circles, and
rectangles on the :code:`Canvas`.  For example:

.. code-block:: C++

    my_canvas.Circle(100, 100, 40, "red", "black");

would draw a circle at (100,100) with a radius of 40, a face color of red, and an outline of
black.

Note also that a number of :code:`Draw()` function exist for :code:`Canvas` that will allow more complex
structures to be drawn easily, such as grids where the colors of each position are
specified.  Several other Empirical tools hook into :code:`Canvas`.  For example if you are building
a world with :code:`emp::Surface`, that surface can be handed to canvas to have all of the shapes
on it (currently just circles) drawn for you.



Other topics, coming soon!

- :code:`TextArea` s
- The Text Widget extras (specifying, closing, modifying, etc.)
- Listeners and Capturing input events
- Working with keypresses
- Customized Divs
- Uploading files
- Interfacing between HTML and Empirical
- Using JSWrap
- Freezing and Activating Widgets
- Animations
