
import json
import runpy
import os.path
import sys
from pathlib import Path

class Pipeline:

    def __init__(self, root_path, args=[]):
        self._tasks = [];
        self._root_path = Path(root_path);
        self._path = self._root_path;
        self.args = args;

    def add(self, task):
        self._tasks.append(task);

    def relativePath(self, path):
        return self._path / path

    def root_dir(self):
        return self._root_path

    def current_dir(self):
        return self._path

    def load(self, another_path):

        print("Loading: " + str(another_path))

        child_file_path = os.path.abspath(self.relativePath(another_path))
        child_dir = os.path.dirname(child_file_path)
        child_name = os.path.splitext(os.path.basename(child_file_path))[0]

        old_path = self._path
        old_sys_path = sys.path;

        self._path = Path(child_dir)
        if sys.path[0] != child_dir:
            sys.path.insert(0,child_dir)

        runpy.run_module(child_name,  { "pipeline": self }, run_name = "")

        self._path = old_path;
        sys.path = old_sys_path

    def save(self, path=None):

        if path is None:
            path = self.defaultSavePath()

        f = open(path, 'w')
        f.write(json.dumps(self._tasks, indent=4))
        f.write('\n')

    def defaultSavePath(self):
        return 'pipeline.json'
