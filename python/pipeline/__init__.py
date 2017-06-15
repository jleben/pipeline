
import json
import runpy
import os.path
import sys

class Pipeline:

    tasks = [];

    def __init__(self, path=None):
        self.tasks = [];
        self.path = path;

    def add(self, task):
        self.tasks.append(task);

    def relativePath(self, path):
        if self.path:
            return os.path.join(self.path, path)
        else:
            return path

    def load(self, path):

        child_file_path = os.path.abspath(self.relativePath(path))
        child_dir = os.path.dirname(child_file_path)
        child_name = os.path.splitext(os.path.basename(child_file_path))[0]

        old_path = self.path
        old_sys_path = sys.path;

        self.path = child_dir
        if sys.path[0] != child_dir:
            sys.path.insert(0,child_dir)

        runpy.run_module(child_name,  { "pipeline": self }, run_name = "")

        self.path = old_path;
        sys.path = old_sys_path

    def save(self, path=None):

        if path is None:
            path = self.defaultSavePath()

        f = open(path, 'w')
        f.write(json.dumps(self.tasks, indent=4))
        f.write('\n')

    def defaultSavePath(self):
        return 'pipeline.json'
