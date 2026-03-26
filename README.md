# Bad Apple with Voronoi diagrams

Instructions:

- Place a video file at `./video_in.mp4`
- Run `make`
- The outputted video file is `./video_out.mp4`.

Improvement steps:
- Optimization ideas: make a thread pool and keep it for the whole program,
  profile to see speed increase potential, parallelize image creation
- Possibility to force do something or force ignore cache file in voronoi step
- Compute bounding boxes for cost evaluation? Worth it for weighted diagrams?
- Fix all TODOs
