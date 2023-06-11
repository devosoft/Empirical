# Empirical Development Practices


```{contents}
```

Although the Empirical project is primarily maintained by the Primary Investigator of the Digital Evolution laboratory at Michigan State University, Dr. Charles Ofria, a large, rotating menagerie of graduate students, undergraduate students, alumni, and collaborators also regularly contribute to the library.
Several active research projects depend on the library.
The scope of the project and the continuous turnover of our developer base have necessitated investment in extensive social and technical tooling on our part.

Our development process revolves around a standard fork and pull request workflow.
We use GitHub actions for continuous integration, ensuring that code merged in to our main branch meets certain quality-control criteria.
We check that merged code does not break existing unit tests or any demonstration code bundled with the repository.
We maintain an extensive unit testing suite, which, as of June 2021, covers 82% of our code base.
We use Codecov to measure code coverage as part of our continuous integration.
This allows us to enforce that merged code provides unit tests for any new content.
In addition to automated quality checks, we enforce manual code reviews on all pull requests.

To help onboard our continuous influx of new library users and new library developers, we maintain documentation [on ReadTheDocs](https://empirical.readthedocs.io/en/latest/) as part of our continuous integration process.
This documentation includes quick start guides, enumeration of our development practices, as well as an automatically-generated API tree with annotated signatures for all functions, classes, and structs.
In order to make writing documentation easier --- especially for new developers --- we recently switched from using restructured text to using markdown with the help of the [MyST library](https://myst-parser.readthedocs.io/en/latest/).

We strive to employ development practices that recognize and confront Diversity, Equity, and Inclusion (DEI) issues.
Our recruitment practices form the foundation of our DEI efforts.
Over the past two years, through the [Workshop for Avida-ED Software Development](https://mmore500.com/waves/) we recruited upwards of 20 undergraduate students for ten weeks of paid contributorship across different aspects of Empirical development and related projects.
More than half of these students identified as members of underrepresented groups in computer science.
This year, we plan to facilitate discussions on DEI topics among the workshop community as part of our core professional enrichment curriculum.

However, we also incorporate a bevvy of more minor technocratic practices motivated by DEI.
We maintain a written Code of Conduct adapted from the [contributor convenant](https://www.contributor-covenant.org/).
We chose to use [EditorConfig](https://editorconfig.org/) to enforce the library style guide because it integrates with our students' code editors through easy-to-install add-ons, allowing them to automatically address minor stylistic concerns.
This practice minimizes arbitrary barriers for new contributors while maintaining acceptable library uniformity.
Finally, we maintain a complete Empirical development environment as a pre-built Docker container hosted on DockerHub.
In the past, we have found dependency installation issues to be a major hindrance to onboarding new developers, especially students that hadn't already been using their machines for extensive software development work.
This practice rolls an otherwise extensive laundry list of tricky installs into a single download, again minimizing arbitrary barriers for new contributors.

A quick note about our python dependencies: in order to prevent build issues due to new package versions, our `requirements.txt` files pin *all* dependencies, including indirect dependencies (i.e., dependencies of our dependencies).
Our direct dependencies are listed (and pinned) in corresponding `requirements.in` files.
To update Empirical's python dependencies, update the `requirements.in` file and `pip-compile requirements.in` to regenerate `requirements.txt`.
Then, commit both files.
