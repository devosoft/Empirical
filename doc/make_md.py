#!/usr/bin/python

import glob
import os

# This script performs preprocessing for the Empirical documentation
# build process, manually filling in the folder tree `library` with stub
# markdown files for the documentation of Empirical header files to be built
# onto.
#
# The script scans the "../include/emp" directory for all ".hpp" files (with
# several explicit exclusions). For each header `emp/subname/filename.hpp`
# it creates a stub `library/subname/api/file.md`. Note that the `library`
# folder already contains skeleton content (i.e., stub pages for each of
# Emprical's subdirectories. Note also that some Empirical subdirectories
# not intended for external use are explicitly excluded.

def format_directive(filename):
    """Create the breathe directive and add the options."""
    directive = f"```{{eval-rst}}\n.. doxygenfile:: {filename}\n   :project: Empirical\n```\n"
    return directive


def format_heading(level, text):
    """Create a heading of <level> [1, 2 or 3 supported]."""
    symbol = ["# ", "## ", "### ", "#### "][
        level - 1
    ]
    return symbol + text + "\n\n"


header_files = glob.glob("../include/emp/**/*.hpp", recursive=True)

for h in header_files:
    print(h)
    h = h.removeprefix("../include/emp/")
    just_dir = "/".join(h.split("/")[:-1]).lower()
    last_dir = just_dir.split("/")[-1]
    just_file = h.split("/")[-1]

    excluded_prefixes = [
        "_",
        "in_progress",
        "polyfill",
    ]
    if any(just_file.startswith(prefix) for prefix in excluded_prefixes):
        continue
    new_dir = "library/" + just_dir + "/api"
    print(new_dir)
    toc_text = "<!-- API TOC -->\n<!-- The above comment tells the API generator that this file has API docs. Don't remove it. -->\n" + format_heading(2, "API") + "```{eval-rst}\n.. toctree::\n   :glob:\n\n   api/*\n```\n"
    os.makedirs(new_dir, exist_ok=True)
    toc_file_name = f"library/{just_dir}/{last_dir}.md"
    if not os.path.exists(toc_file_name):
        toc_text = format_heading(1, last_dir.capitalize()) + toc_text
        with open(toc_file_name, "w") as target:
            target.write(toc_text)
    elif "<!-- API TOC -->" not in open(toc_file_name).read():
        with open(toc_file_name, "a") as target:
            target.write("\n" + toc_text)

    text = format_heading(1, f"{just_file}")
    text += format_directive("emp/" + h)

    filename = just_file.removesuffix(".hpp") + ".md"

    with open(new_dir + "/" + filename, "w") as target:
        target.write(text)
