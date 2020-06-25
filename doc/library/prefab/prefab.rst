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

.. code-block:: c++
    #include "prefab/Card.h"

    emp::prefab::Card my_card(emp::prefab::Card::Collapse::NONE);
    my_card.AddHeaderContent("Title");
    my_card.AddBodyContent("Content for the card's body");
    // Web components can also be passed as parameters to AddHeaderContent and AddBodyContent

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

.. code-block:: c++
    #include "prefab/Collapse.h"
    #include "web/Div.h"

    // Fill these divs with content
    emp::web::Div link_ele;
    emp::web::Div toggle_ele;

    emp::prefab::Collapse my_collapse(link_ele, toggle_ele, true);

CommentBox
~~~~~~~~~~
A CommentBox is a simple grey comment bubble. Content can be added to it using 
the AddContent method. If there is data you only want to be visible on mobile
devices, used the AddMobileContent method.

Since this class inherits from web::Div, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your CommentBox into other web
components with the << operator.

Example:

.. code-block:: c++
    #include "prefab/CommentBox.h"

    emp::prefab::CommentBox my_box;
    my_box.AddContent("<h1>Content that shows on all screen sizes</h1>"); 
    my_box.AddMobileContent("<hr>Content that only shows on small screens");
    // Web components can also be passed as parameters to AddContent and AddMobileContent

ConfigPanel
~~~~~~~~~~~
Using the ConfigPanel class, a configuration panel is constructed when passed a Config file. It uses other
Prefabricated components to add styling and structure to the panel. Use the GetDiv 
method to stream this component into another web component or document.

Example:

.. code-block:: c++
    #include "prefab/ConfigPanel.h"
    #include "config/ArgManager.h"
    #include "web/web.h"

    #include "SampleConfig.h" // Config file

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

FontAwesomeIcon
~~~~~~~~~~~~~~~
`FontAwesome <https://fontawesome.com/v4.7.0/>`__ is a free library of icons. This class, find 
the icon you want to use from the `icons page <https://fontawesome.com/v4.7.0/icons/>`___. 
Pass "fa-" + *icon name* as a parameter to the constructor. 

Since this class inherits from web::Element, you can set styling and attributes
with SetCSS and SetAttr respectively. You can also stream your FontAwesomeIcon into other web
components with the << operator.

Example:

.. code-block:: c++
    #include "prefab/FontAwesomeIcon.h"

    emp::prefab::FontAwesomeIcon my_icon("fa-paw");