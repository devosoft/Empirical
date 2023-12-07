import glob
import os


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


header_files = glob.glob("../include/emp/**/*.hpp")

for h in header_files:
    print(h)
    h = h.removeprefix("../include/emp/")
    just_dir = "/".join(h.split("/")[:-1]).lower()
    last_dir = just_dir.split("/")[-1]
    just_file = h.split("/")[-1]
    if just_file.startswith("_"):
        continue
    if just_dir.startswith("in_progress"):
        continue
    if just_dir.startswith("polyfill"):
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
