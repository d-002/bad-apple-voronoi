# Bad Apple with Voronoi diagrams

Instructions:

- Place a video file at `./video_in.mp4`
- Run `make`
- The outputted video file is `./video_out.mp4`.

Improvement steps:
- Implement state save/load for shared_data and current image index, check cache
  integrity compared to the modification timestamp of the state file
- Remove cache file in make clean, possibility to force do something or force
  ignore cache file
- Compute bounding boxes for cost evaluation? Worth it for weighted diagrams?
- Paralellize computing parts of the gradient / use the GPU for cost computation
- Fix all TODOs
