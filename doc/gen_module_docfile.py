# currently, hardcode a list of files and then spew them out in an rst looking
# kinda thing


entry = """
**{filename}**

.. doxygenfile:: {filename}
   :project: Empirical

"""

with open('file_list', 'r') as listfile:
    content = listfile.readlines()
    with open('docfile.rst', 'w') as docfile:
        for line in content:
            outline = entry.format(filename=line.strip())
            docfile.writelines(outline)
