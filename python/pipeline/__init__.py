import json

class Pipeline:

    def __init__(self, path=None):
        self.tasks = [];

    def add(self, task):
        self.tasks.append(task);

    def save(self, path=None):

        if path is None:
            path = 'pipeline.json'

        f = open(path, 'w')
        f.write(json.dumps(self.tasks))
        f.write('\n')

