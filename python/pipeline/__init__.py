
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

    def relativePath(self, path):
        if self.path:
            return self.path + "/" + path
        else:
            return path

    def load(self, path):

        current_path = self.path

        child_file_path = self.relativePath(path)

        self.path = os.path.dirname(child_file_path)

        runpy.run_path(child_file_path,  { "pipeline": self })

    def save(self, path=None):

        if path is None:
            path = self.defaultSavePath()

        f = open(path, 'w')
        f.write(json.dumps(self.tasks))
        f.write('\n')

    def defaultSavePath(self):
        return 'pipeline.json'
