import json
import sys

class Pipeline:

    def __init__(self):
        self.tasks = [];

    def add(self, task):
        self.tasks.append(task);

    def save(self, path=None):

        if path is None:
            path = self.defaultPath()

        f = open(path, 'w')
        f.write(json.dumps(self.tasks))
        f.write('\n')

    def defaultPath(self):
        if len(sys.argv) > 1:
            return sys.argv[1]
        else:
            return 'pipeline.json'
