# Bad Apple with Voronoi diagrams

Instructions:

- Place a video file at `./video_in.mp4`
- Run `make`
- The outputted video file is `./video_out.mp4`.

Improvement steps:
- New cost evaluation: use JFA for base cost, then for secondary cost compute
  the centroid of a cell and use `sqrt(dist2(cell, centroid)) / max(w, h)`
- Better ETA calculation with last 10 iterations, store time 10 iterations ago
  and compute based on that
- Optimization ideas: make a thread pool and keep it for the whole program,
  profile to see speed increase potential, parallelize image creation
- Rendering idea: smoothing in between frames, possible replacement to %
- Possibility to force do something or force ignore cache file in voronoi step
- Style idea: choose the color that is the least present in the image (store in
  shared data, change this using hysteresis) and use that to figure out which
  color of cells to not display borders/centers
- Fix all TODOs
