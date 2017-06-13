import json
import runpy
import os.path

class Pipeline:

    tasks = [];

    def __init__(self, path=None):
        self.tasks = [];
        self.path = path;

    def add(self, task):
        self.tasks.append(task);

    def save(self, path=None):

        if path is None:
            path = self.defaultSavePath()

        f = open(path, 'w')
        f.write(json.dumps(self.tasks))
        f.write('\n')

    def defaultSavePath(self):
        return 'pipeline.json'

    def relativePath(self, path):
        if self.path:
            return self.path + "/" + path
        else:
            return path

    def load(self, path):
        p = Pipeline(self.relativePath(os.path.dirname(path)))

        namespace = runpy.run_path(path,  { "pipeline": p })

        if isinstance(p, Pipeline):
            self.tasks += p.tasks
