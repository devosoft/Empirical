Prefabricated Web Tools (for use with Emscripten)
=================================================

These prefabricated tools were created to help you quickly create interesting web applicications without being overwhelmed with the underlying HTML, CSS, and Bootstrap classes required.
These tools use Empirical's web tools to provide structure for the site, and many of the prefab tools inherit from web tools so you can add your own styling and stream them into other web components in a similar way.

When using these prefab tools be sure to link to the Bootstrap library, jQuery, and the default style stylesheet for this class in the head section of your HTML file.
.. code-block:: html

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css">
  <link rel="stylesheet/less" type="text/css" href="https://cdn.jsdelivr.net/gh/devosoft/Empirical@master/include/emp/prefab/DefaultPrefabStyles.less">
  <script src="//cdn.jsdelivr.net/npm/less" ></script>
  <script src="jquery-1.11.2.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.3/umd/popper.min.js" integrity="sha384-vFJXuSJphROIrBnz7yo7oB41mKfc8JzQZiCq4NCceLEaO4IHwicKwpJf9c9IpFgh" crossorigin="anonymous"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/js/bootstrap.min.js" integrity="sha384-alpBpkh1PFOepccYVYDB4do5UnbKysX5WZXm3XxPqe5iKTfUKjNkCk9SaVuEZflJ" crossorigin="anonymous"></script>

You can view these tools in action `here <https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo>`_.

Card
~~~~
The Card class allows you to define a Bootstrap style card into your project.
A card that is not collapsible will have its state set to :code:`STATIC`.
Cards are static by default.
A card can be collapsible if its state parameter it set to :code:`INIT_OPEN` or :code:`INIT_CLOSED`.
By default, if a card is collapsible, it will have toggle icons in the header, but this can be overridden by setting the :code:`showGlyphs` parameter to :code:`false`.

Since this class inherits from :code:`web::Div`, you can set styling and attributes with :code:`SetCSS` and :code:`SetAttr` respectively.
You can also stream your Card into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: c++

  #include "web/web.h"
  #include "prefab/Card.h"

  emp::web::Document doc("emp_base");

  emp::prefab::Card my_card("STATIC");
  doc << my_card;

  my_card.AddHeaderContent("Title");
  my_card.AddBodyContent("Content for the card's body");
  // Web components can also be passed as parameters to AddHeaderContent and AddBodyContent

**Note**: The toggle icons that are avalible for collapsible cards use the `FontAwesome`_ library.
You will need to add the CSS file for this library to the head of your HTML file:

.. code-block:: html

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">

CodeBlock
~~~~~~~~~
The CardBlock class provides an interface for the `HighlightJS Library`_ which allows you to display code on web pages with language specific highlighting.
You can find a list of `all languages`_ on their GitHub page.

To use this class, you need to pass the code you want displayed and the programming language to the constructor.

Since this class inherits from :code:`web::Element`, you can stream your CodeBlock into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: c++

  #include "web/web.h"
  #include "prefab/CodeBlock.h"

  emp::web::Document doc("emp_base");

  std::string code_str =
    R"(
      int num = 9;
      std::cout << num << " is a square number" << std::endl;
    )";
  emp::prefab::CodeBlock code_block(code_str, "c++");

  doc << code_block;

**Note**: You will also need to add the following code to the bottom of the body section of your HTML file:

.. code-block:: html

  <link rel="stylesheet" href="//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.0/styles/default.min.css">
  <script src="//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.0/highlight.min.js"></script>
  <script src="https://cdn.jsdelivr.net/gh/devosoft/Empirical@master/include/emp/prefab/HighlightJS.js"></script>


.. _HighlightJS Library: https://highlightjs.org/
.. _all languages: https://github.com/highlightjs/highlight.js/blob/master/SUPPORTED_LANGUAGES.md

Collapse
~~~~~~~~
The CollapseCouple maintains a group of targets and controllers.
When a controller is clicked on a web page, all the associated targets will change state (expand/collapse).

By default, the target element will start off closed, but this can be set to open by passing :code:`true` for the :code:`expanded` parameter.

Since the collapse controller and collapse target element will not necessarily directly neighbor eachother, call :code:`GetControllerDiv()` and :code:`GetTargetDiv()` to obtain a vector of all the asspociated controllers and targets, respectively.
To obtain just one controller or target, pass its index into a get div function call.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "web/Div.h"
  #include "prefab/CommentBox.h"

  #include "prefab/Collapse.h"

  emp::web::Document doc("emp_base");

  emp::prefab::CommentBox box1;
  box1.AddContent("<h3>Box 1</h3>");
  emp::web::Div btn1;
  btn1.SetAttr("class", "btn btn-info");
  btn1 << "Button 1: controls box 1";

  emp::prefab::CollapseCoupling collapse1(btn1, box1, true);

  doc << collapse1.GetControllerDiv(0);
  doc << collapse1.GetTargetDiv(0);

CommentBox
~~~~~~~~~~
A CommentBox is a simple grey comment bubble.
Content can be added to it using :code:`AddContent()`.
If there is data you only want to be visible on mobile devices, use :code:`AddMobileContent()`.

Since this class inherits from :code:`web::Div`, you can set styling and attributes with :code:`SetCSS()` and :code:`SetAttr()` respectively.
You can also stream your CommentBox into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "prefab/CommentBox.h"

  emp::web::Document doc("emp_base");

  emp::prefab::CommentBox my_box;
  doc << my_box;

  my_box.AddContent("<h1>Content that shows on all screen sizes</h1>");
  my_box.AddMobileContent("<hr>Content that only shows on small screens");
  // Web components can also be passed as parameters to AddContent and AddMobileContent

ConfigPanel
~~~~~~~~~~~
The ConfigPanel allows developers to easily set up a user interface for their configuration options.
It allows web apps to be interactive and dynamic, allowing users to change configuration settings within the applicaiton and providing a better user experiance.

Using the ConfigPanel class, a configuration panel is constructed when passed a Config file.
It uses other Prefabricated components to add styling and structure to the panel.
Use :code:`GetConfigPanelDiv()` to stream this component into another web component or document.

It is important to note that ConfigPanel instances are destroyed when they go out of scope.
This causes the form to no longer respond to changes made by the user.
You will need to initialize an instance outside of :code:`main()` if you would like the user to be able to interact with the panel.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "prefab/ConfigPanel.h"
  #include "config/ArgManager.h"

  #include "SampleConfig.h" // Config file

  emp::web::Document doc("emp_base");
  Config cfg;

  emp::prefab::ConfigPanel config_panel(cfg);

  // apply configuration query params and config files to Config
  auto specs = emp::ArgManager::make_builtin_specs(&cfg);
  emp::ArgManager am(emp::web::GetUrlParams(), specs);
  // cfg.Read("config.cfg");
  am.UseCallbacks();
  if (am.HasUnused()) std::exit(EXIT_FAILURE);

  // setup configuration panel
  config_panel.Setup();
  doc << config_panel.GetConfigPanelDiv();

FontAwesomeIcon
~~~~~~~~~~~~~~~
`FontAwesome`_ is a free library of icons that can be used in web pages.

To use this class:

1. Find the icon you wish to use in the `FontAwesome library`_
2. Pass :code:`"fa-" + *icon name*` as a parameter to the constructor.
3. Add the following CSS file to the head of your HTML document.

.. code-block:: html

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">

Since this class inherits from :code:`web::Element`, you can set styling and attributes with :code:`SetCSS()` and :code:`SetAttr()` respectively.
You can also stream your FontAwesomeIcon into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "prefab/FontAwesomeIcon.h"

  emp::web::Document doc("emp_base");

  emp::prefab::FontAwesomeIcon my_icon("fa-paw");
  doc << my_icon;

  my_icon.AddClass("custom_class");

.. _FontAwesome: https://fontawesome.com/v4.7.0/
.. _FontAwesome library: https://fontawesome.com/v4.7.0/icons/

LoadingIcon
~~~~~~~~~~~
The LoadingIcon class is used to add an animated loading icon.
One possible use for this icon is to be displayed while the contents of a web page is loading.
The icon is provided by `FontAwesome`_, so you will need to add its CSS to your HTML file to use this class.

.. code-block:: html

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">

Since this class inherits from :code:`web::Element`, you can set styling and attributes with :code:`SetCSS()` and :code:`SetAttr()` respectively.
You can also stream your LoadingIcon into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "prefab/LoadingIcon.h"

  emp::web::Document doc("emp_base");

  emp::prefab::LoadingIcon spinner;
  doc << spinner;

LoadingModal
~~~~~~~~~~~~
The LoadingModal header file makes adding a loading modal to a web page easy.
It will appear while the content of the page is rendering and will disappear when the page has completed loading.

This header file is slightly different from the other prefab web tools.
To place the loading modal on your web page, you must import the LoadingModal.js script into your HTML file right after the opening body tag.
To close the modal you must call the :code:`CloseLoadingModal()` function in your .cc file after loading the desired content into the doc.

Example:
********
.. code-block:: cpp

  // .cc file
  #include "web/web.h"
  #include "LoadingModal.h"

  emp::web::Document doc("emp_base");

  // Add elements to the doc a normal

  emp::prefab::CloseLoadingModal();

.. code-block:: html

  <!-- HTML file -->
  <html>
  <head>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css">
    <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/gh/devosoft/Empirical@master/include/emp/prefab/DefaultPrefabStyles.less">

    <script src="jquery-1.11.2.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.3/umd/popper.min.js" integrity="sha384-vFJXuSJphROIrBnz7yo7oB41mKfc8JzQZiCq4NCceLEaO4IHwicKwpJf9c9IpFgh" crossorigin="anonymous"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/js/bootstrap.min.js" integrity="sha384-alpBpkh1PFOepccYVYDB4do5UnbKysX5WZXm3XxPqe5iKTfUKjNkCk9SaVuEZflJ" crossorigin="anonymous"></script>
  </head>
  <body>
    <!-- Loading Modal JS -->
    <script src="https://cdn.jsdelivr.net/gh/devosoft/Empirical@master/include/emp/prefab/LoadingModal.js"></script>

    <!-- Rest of body section -->
  </body>
  </html>

Modal
~~~~~
The Modal class can be used to create Bootstrap modals that pops up in the middle of the screen.

Since this class inherits from :code:`web::Div`, you can stream your Modal into other web components with the :code:`<<` operator.
You can also set the background color of the Modal with :code:`SetBackground()` passing it a string with a color name or its hex code value.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "web/Button.h"
  #include "prefab/Modal.h"

  emp::web::Document doc("emp_base");

  emp::prefab::Modal modal;
  doc << modal;

  modal.AddHeaderContent("<h3>Modal Header Section</h3>");
  modal.AddBodyContent("This is the content of the modal");

  modal.AddFooterContent("Modal Footer Section");
  UI::Button close_btn([](){;}, "Close");
  close_btn.SetAttr("class", "btn btn-secondary");
  modal.AddFooterContent(close_btn);
  modal.AddButton(close_btn);

  modal.AddClosingX();

  UI::Button modal_btn([](){;}, "Show Modal");
  doc << modal_btn;
  modal_btn.SetAttr("class", "btn btn-info");
  modal.AddButton(modal_btn);

ToggleSwitch
~~~~~~~~~~~~
The ToggleSwitch class wraps checkbox input with Bootstrap custom swtich classes.
If you need to add a CSS class to the Input, do it after the creating the ToggleSwitch instance with :code:`AddClass()`.


Since this class inherits from :code:`web::Element`, you can set styling and attributes with :code:`SetCSS()` and :code:`SetAttr()` respectively.
You can also stream your ToggleSwitch into other web components with the :code:`<<` operator.

Example:
********
.. code-block:: cpp

  #include "web/web.h"
  #include "prefab/ToggleSwitch.h"

  emp::prefab::ToggleSwitch on_switch([](std::string val){}, "Switch Defult On", true, "user_defined_switch_id");
  doc << on_switch;

  doc << "<br>";

  emp::prefab::ToggleSwitch off_switch([](std::string val){}, NULL, false);
  doc << off_switch;
  off_switch.AddLabel("Switch Defult Off");

Add the link to Bootstrap in the head of your HTML file:
.. code-block:: html

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css">
