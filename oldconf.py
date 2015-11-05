import sys, os

sys.path.append(os.path.abspath('../third-party/breathe'))

extensions = ['sphinx.ext.pngmath', 'sphinx.ext.todo', 'breathe']

breathe_projects = { "Empirical": os.path.abspath('../build/doxydoc/xml/')}
