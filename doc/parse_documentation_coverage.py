# adapted from https://github.com/mmore500/conduit/blob/master/ci/parse_documentation_coverage.py

import json
import sys

with open(sys.argv[1]) as json_file:
    data = json.load(json_file)
    cov = float(data['total']['coverage_rate'])
    color = "red"
    if cov > .9:
        color = "brightgreen"
    elif cov > .8:
        color = "green"
    elif cov > .7:
        color = "yellowgreen"
    elif cov > .5:
        color = "yellow"
    elif cov > .25:
        color = "orange"

    res = {
      'schemaVersion': 1,
      'label': 'documentation coverage',
      'message': '{:.1%}'.format(cov),
      'color': color,
    }
    print(json.dumps(res, sort_keys=True, indent=4))
