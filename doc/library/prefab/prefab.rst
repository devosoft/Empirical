Prefabricated Web Tools (for use with Emscripten)
=================================================

These prefabricated tools were created to help you quickly create interesting
web applicications without being overwhelmed with the underlying HTML, CSS, and 
Bootstrap classes required. These tools use Empirical's web tools to provide 
structure for the site, and many of the prefab tools inherit from web tools so you can add your
own styling and stream them into other web components in a similar way.

You can view these tools in action `here <https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo>`_.

Card
~~~~
The Card class allows you to define a Bootstrap style card into your
project. A card can be collapsible if it's state parameter it set to OPEN or CLOSED.
By default, if a card is collapsible, it will have toggle icons in the header,
but this can be overridden by setting the showGlyphs parameter to false.

Since this class inherits from web::Div, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your Card into other web
components with the << operator.

Example:
********
.. code-block:: c++

    #include "web/web.h"
    #include "prefab/Card.h"

    emp::web::Document doc("emp_base");

    emp::prefab::Card my_card(emp::prefab::Card::Collapse::NONE);
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
The CardBlock class is an interface for `highlightjs`_ which allows you to display code on 
web pages with language specific highlighting. You can find a list of `all languages`_ on
their GitHub page.

To use this class you need to pass the code you want displayed and the programming language
to the constructor. 

Since this class inherits from web::Element, you can stream your CodeBlock into other web
components with the << operator.

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
    <script>hljs.initHighlightingOnLoad();</script>
    
.. _highlightjs: https://highlightjs.org/
.. _all languages: https://github.com/highlightjs/highlight.js/blob/master/SUPPORTED_LANGUAGES.md
    
Collapse
~~~~~~~~
The Collapse class is used to create collapsible elements in the document. It requires a web
component to be the element that controls the expaning and collapsing of the another element on
the page. The element that will expand and collapse can be passed to the constructor if it is not
controlled by another linking element already. Otherwise, only providing the linking element and 
the id of the collapseing element is necessary.

By default, the toggle element will be closed, but this can be set to open by passing true for the
expanded parameter.

Since the linking and collapsing element will not necessarily be neighboring on the page, call 
GetLinkDiv() to obtain the HTML for the link element and GetToggleDiv() to obtain the HTML
for the toggle element.

Example:
********
.. code-block:: cpp

    #include "web/web.h"
    #include "web/Div.h"
    #include "prefab/Collapse.h"

    emp::web::Document doc("emp_base");

    // Fill these divs with content
    emp::web::Div link_ele;
    emp::web::Div toggle_ele;

    emp::prefab::Collapse my_collapse(link_ele, toggle_ele, true);

    doc << my_collapse.GetLinkDiv();
    doc << my_collapse.GetToggleDiv();

CommentBox
~~~~~~~~~~
A CommentBox is a simple grey comment bubble. Content can be added to it using 
the AddContent method. If there is data you only want to be visible on mobile
devices, used the AddMobileContent method.

Since this class inherits from web::Div, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your CommentBox into other web
components with the << operator.

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
Using the ConfigPanel class, a configuration panel is constructed when passed a Config file. It uses other
Prefabricated components to add styling and structure to the panel. Use the GetDiv 
method to stream this component into another web component or document.

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
    doc << config_panel.GetDiv();

FontAwesomeIcon
~~~~~~~~~~~~~~~
`FontAwesome`_ is a free library of icons that can be used in web pages.

To use this class:

1. Find the icon you wish to use in the `FontAwesome library`_
2. Pass "fa-" + *icon name* as a parameter to the constructor.
3. Add the following CSS file to the head of your HTML document.

.. code-block:: html

    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">
    
Since this class inherits from web::Element, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your FontAwesomeIcon into other web
components with the << operator.

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
The LoadingIcon class is used to add an animated loading icon. One possible use
for this icon is to be displayed while the contents of a web page is loading. The icon
is provided by `FontAwesome`_, so you will need to add its CSS to your HTML file to use
this class.

.. code-block:: html

    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">
  
Since this class inherits from web::Element, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your LoadingIcon into other web
components with the << operator.

Example:
********
.. code-block:: cpp

    #include "web/web.h"
    #include "prefab/LoadingIcon.h"

    emp::web::Document doc("emp_base");

    emp::prefab::LoadingIcon spinner;
    doc << spinner;

ToggleSwitch
~~~~~~~~~~~~
This class is a wrapper for a checkbox input. It uses Bootstrap 4.5.0 to create a 
custom toggle switch. To create a ToggleSwitch instance, an Input widget must be 
passed as a parameter. If you need to add a CSS class to the Input, do it after the creating
the ToggleSwitch instance with AddClass().


Since this class inherits from web::Element, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your ToggleSwitch into other web
components with the << operator.

Example:
********
.. code-block:: cpp

    #include "web/web.h"
    #include "web/Input.h"
    #include "prefab/ToggleSwitch.h"

    emp::web::Document doc("emp_base");

    emp::web::Input input_element(
      [](std::string str){;},
      "checkbox", NULL, "input_id"
    );
    emp::prefab::ToggleSwitch my_switch(input_element);
    doc << my_switch;

    my_switch.AddLabel("Switch Label");

Add the link to Bootstrap in the head of your HTML file:
.. code-block:: html

    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css">
