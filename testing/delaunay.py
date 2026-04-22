import numpy as np
import matplotlib.pyplot as plt
from typing import Any, cast

class Edge:
    def __init__(self, a: np.ndarray, b: np.ndarray) -> None:
        self.a, self.b = a, b
        self.connected_triangles: list[int] = [] # will be filled later

    def __eq__(self, other: Any) -> bool:
        if not isinstance(other, Edge):
            return False
        eq = np.all(self.a == other.a) and np.all(self.b == other.b)
        eq |= np.all(self.a == other.b) and np.all(self.b == other.a)
        return cast(bool, eq)

    def __repr__(self) -> str:
        return f'Edge<{self.a} {self.b}>'

class CommonEdge:
    def __init__(self, edge: Edge) -> None:
        self.edge = edge
        self.triangles: list[Triangle] = []

    def __repr__(self) -> str:
        return f'CommonEdge<{self.edge}>'

class VoronoiEdge:
    def __init__(self, edge: Edge, cells: tuple[np.ndarray, np.ndarray]
                 ) -> None:
        self.edge = edge
        self.cells = cells

    def __repr__(self) -> str:
        return f'VoronoiEdge<{self.edge} {self.cells}>'

class VoronoiPolygon:
    def __init__(self) -> None:
        self.edges: list[VoronoiEdge] = []

    def __repr__(self) -> str:
        return f'VoronoiPolygon<{self.edges}>'

class Triangle:
    def __init__(self, edges: list[Edge]) -> None:
        self.edges = edges
        self.points = np.array([e.a for e in edges])

    def __repr__(self) -> str:
        return f'Triangle<{self.points}>'

    def draw(self, plt, **kwargs) -> None:
        x = list(self.points[:, 0])
        y = list(self.points[:, 1])
        x.append(self.points[0][0])
        y.append(self.points[0][1])
        plt.plot(x, y, **kwargs)

def inside_circumcircle(triangle: Triangle, point: np.ndarray) -> bool:
    (ax, ay), (bx, by), (cx, cy) = triangle.points
    dx, dy = point
    ccw = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay) > 0

    ax_ = ax - dx;
    ay_ = ay - dy;
    bx_ = bx - dx;
    by_ = by - dy;
    cx_ = cx - dx;
    cy_ = cy - dy;

    det = (
        (ax_ * ax_ + ay_ * ay_) * (bx_ * cy_-cx_ * by_) -
        (bx_ * bx_ + by_ * by_) * (ax_ * cy_-cx_ * ay_) +
        (cx_ * cx_ + cy_ * cy_) * (ax_ * by_-bx_ * ay_)
    )

    return (det < 0) ^ ccw

def edge_in(triangles: list[Triangle], ignore: Triangle, edge: Edge) -> bool:
    for triangle in triangles:
        if triangle == ignore:
            continue
        if edge in triangle.edges:
            return True
    return False

def includes_super_point(triangle: Triangle, super: Triangle) -> bool:
    for point in triangle.points:
        if point in super.points:
            return True

    return False

def bowyer_watson(bounds: np.ndarray, cells: np.ndarray) -> list[Triangle]:
    x0, y0 = bounds[0]
    w, h = bounds[1] - bounds[0]
    p0 = np.array([x0 - w, y0])
    p1 = np.array([x0 + 2 * w, y0])
    p2 = np.array([x0 + .5 * w, y0 + 1.5 * h])
    super_triangle = Triangle([Edge(p0, p1), Edge(p1, p2), Edge(p2, p0)])
    triangulation = [super_triangle]

    for cell in cells:
        bad_triangles = []
        for triangle in triangulation:
            if inside_circumcircle(triangle, cell):
                bad_triangles.append(triangle)

        polygon = []
        for triangle in bad_triangles:
            for edge in triangle.edges:
                if not edge_in(bad_triangles, triangle, edge):
                    polygon.append(edge)

        for triangle in bad_triangles:
            triangulation.remove(triangle)

        for edge in polygon:
            p0, p1, p2 = edge.a, edge.b, cell
            triangle = Triangle([edge, Edge(p1, p2), Edge(p2, p0)])
            triangulation.append(triangle)

    for triangle in triangulation[:]:
        if includes_super_point(triangle, super_triangle):
            triangulation.remove(triangle)

    return triangulation

def find_circumcenter(triangle: Triangle) -> np.ndarray:
    (ax, ay), (bx, by), (cx, cy) = triangle.points

    D = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by))
    if np.isclose(D, 0):
        print(f'SILENT ERROR: null determinant for triangle {triangle.points}')

    x = 1 / D * ((ax * ax + ay * ay) * (by - cy)
                 + (bx * bx + by * by) * (cy - ay)
                 + (cx * cx + cy * cy) * (ay - by))
    y = 1 / D * ((ax * ax + ay * ay) * (cx - bx)
                 + (bx * bx + by * by) * (ax - cx)
                 + (cx * cx + cy * cy) * (bx - ax))

    return np.array([x, y])

def get_out_edge(bounds: np.ndarray, center: np.ndarray, edge: Edge) -> Edge:
    (x0, y0), (x1, y1) = bounds

    # find the ray direction
    shift = (edge.a + edge.b) / 2 - center
    shift /= np.linalg.norm(shift)

    # stop at the closest edge
    x, y = np.abs(shift)
    cx, cy = center
    lr_dist = np.array([cx - x0, x1 - cx])
    td_dist = np.array([cy - y0, y1 - cy])
    if lr_dist.min() * y < td_dist.min() * x:
        # will hit left/right walls earlier
        if lr_dist[0] < lr_dist[1]:
            if shift[0] > 0:
                shift *= -1
            endpoint = center + shift * lr_dist[0] / x
            endpoint[0] = x0
        else:
            if shift[0] < 0:
                shift *= -1
            endpoint = center + shift * lr_dist[1] / x
            endpoint[0] = x1
    else:
        # will hit top/bottom walls earlier
        if td_dist[0] < td_dist[1]:
            if shift[1] > 0:
                shift *= -1
            endpoint = center + shift * td_dist[0] / y
            endpoint[1] = y0
        else:
            if shift[1] < 0:
                shift *= -1
            endpoint = center + shift * td_dist[1] / y
            endpoint[1] = y1

    return Edge(center, endpoint)

def to_voronoi_edges(bounds: np.ndarray,
                     triangulation: list[Triangle]) -> list[VoronoiEdge]:
    circumcenters = np.array([find_circumcenter(triangle)
                              for triangle in triangulation])
    (x0, y0), (x1, y1) = bounds
    X, Y = circumcenters.T
    active = (x0 <= X) & (X <= x1) & (y0 <= Y) & (Y <= y1)

    edges_pool = []
    for i, triangle in enumerate(triangulation):
        if not active[i]:
            continue

        for edge in triangle.edges:
            if edge in edges_pool:
                # will need optimizing / changes
                edge = edges_pool[edges_pool.index(edge)]
            else:
                edges_pool.append(edge)
            edge.connected_triangles.append(i)

    voronoi_edges = []

    for edge in edges_pool:
        conn_tri = edge.connected_triangles

        # bounds: extend a ray
        if len(conn_tri) == 1:
            index = conn_tri[0]
            out_edge = get_out_edge(bounds, circumcenters[index], edge)
            voronoi_edges.append(VoronoiEdge(out_edge, (edge.a, edge.b)))
            continue

        # in between two triangles: boundary between cells
        voronoi_edges.append(VoronoiEdge(
            Edge(*circumcenters[conn_tri[:2]]), (edge.a, edge.b)))

    return voronoi_edges

def to_voronoi_polygons(bounds: np.ndarray, cells: np.ndarray,
                        voronoi_edges: list[VoronoiEdge]
                        ) -> list[VoronoiPolygon]:
    polygons = [VoronoiPolygon() for _ in range(len(cells))]

    def get_index(mat: np.ndarray, elt: np.ndarray):
        for i, x in enumerate(mat):
            if np.all(x == elt):
                return i

        raise ValueError('get_index: not found')

    for edge in voronoi_edges:
        for cell in edge.cells:
            # todo optimize for O(N ln N) (at least in C code)
            i = get_index(cells, cell)
            polygons[i].edges.append(edge)

    return polygons

# [[x0, y0], [x1, y1]]
bounds = np.array([[-10, -10], [10, 10]])
orig, ranges = bounds[0], bounds[1] - bounds[0]
N = 100
cells = np.array([np.random.rand(2) * ranges + orig for _ in range(N)])
print('Making triangulation...')
triangulation = bowyer_watson(bounds, cells)
print('Finding edges...')
voronoi_edges = to_voronoi_edges(bounds, triangulation)
print('Building polygons...')
voronoi_polygons = to_voronoi_polygons(bounds, cells, voronoi_edges)
print('Done.')

for triangle in triangulation:
    triangle.draw(plt, color='lightgray')
for edge in voronoi_edges:
    x = np.array([edge.edge.a[0], edge.edge.b[0]])
    y = np.array([edge.edge.a[1], edge.edge.b[1]])
    plt.plot(x, y, color='black')
for polygon in voronoi_polygons:
    arr = np.array([edge.edge.a for edge in polygon.edges])
    plt.fill(arr, alpha=.2)
plt.scatter(*cells.T, color='blue')

plt.show()
