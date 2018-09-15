
import json
import runpy
import os.path
import sys
from pathlib import Path

class Pipeline:

    tasks = [];

    def __init__(self, args=[]):
        self.tasks = [];
        self.path = None;
        self.args = args;

    def add(self, task):
        self.tasks.append(task);

    def relativePath(self, path):
        if self.path is None:
            return Path(path)
        else:
            return self.path / path

    def load(self, another_path):

        child_file_path = os.path.abspath(self.relativePath(another_path))
        child_dir = os.path.dirname(child_file_path)
        child_name = os.path.splitext(os.path.basename(child_file_path))[0]

        old_path = self.path
        old_sys_path = sys.path;

        self.path = Path(child_dir)
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
