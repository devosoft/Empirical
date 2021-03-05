# from https://github.com/mmore500/conduit/blob/master/ci/parse_documentation_coverage.py

import json
import sys

with open(sys.argv[1]) as json_file:
    data = json.load(json_file)
    res = {
      'schemaVersion' : 1,
      'label' : 'documentation coverage',
      'message' : '{:.1%}'.format(float(data['total']['coverage_rate'])),
      'color' : 'orange',
    }
    print(json.dumps(res, sort_keys=True, indent=4))