
pipeline.add({
    "name": "Subdir Test",
    "command": "echo {}".format(pipeline.relativePath("dummy"))
})
