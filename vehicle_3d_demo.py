#!/usr/bin/env python3
"""
Basic 3D vehicle visualization using ASCII art and simple text rendering
Shows the vehicle with weapons we created for Interstate3D
"""

import math
import sys

def load_obj_vertices(filename):
    """Load vertices from OBJ file"""
    vertices = []
    faces = []
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                if line.startswith('v '):
                    parts = line.strip().split()
                    vertices.append([float(parts[1]), float(parts[2]), float(parts[3])])
                elif line.startswith('f '):
                    parts = line.strip().split()
                    face = []
                    for part in parts[1:]:
                        vertex_idx = int(part.split('/')[0]) - 1
                        face.append(vertex_idx)
                    faces.append(face)
    except:
        print(f"Could not load {filename}")
        return [], []
    
    return vertices, faces

def rotate_point(point, angle_x, angle_y, angle_z):
    """Rotate a 3D point"""
    x, y, z = point
    
    # Rotate around X axis
    cos_x, sin_x = math.cos(angle_x), math.sin(angle_x)
    y_new = y * cos_x - z * sin_x
    z_new = y * sin_x + z * cos_x
    y, z = y_new, z_new
    
    # Rotate around Y axis
    cos_y, sin_y = math.cos(angle_y), math.sin(angle_y)
    x_new = x * cos_y + z * sin_y
    z_new = -x * sin_y + z * cos_y
    x, z = x_new, z_new
    
    # Rotate around Z axis
    cos_z, sin_z = math.cos(angle_z), math.sin(angle_z)
    x_new = x * cos_z - y * sin_z
    y_new = x * sin_z + y * cos_z
    x, y = x_new, y_new
    
    return [x, y, z]

def project_to_2d(point, width=80, height=40):
    """Project 3D point to 2D screen coordinates"""
    x, y, z = point
    
    # Simple perspective projection
    if z + 5 > 0:
        scale = 10 / (z + 5)
        screen_x = int(width/2 + x * scale * 10)
        screen_y = int(height/2 - y * scale * 10)
        return screen_x, screen_y
    return None

def render_ascii_3d(vertices, faces, offset=[0, 0, 0], rotation=[0, 0, 0], char='@'):
    """Render 3D mesh as ASCII art"""
    width, height = 80, 40
    screen = [[' ' for _ in range(width)] for _ in range(height)]
    
    # Transform and project vertices
    projected_points = []
    for vertex in vertices:
        # Apply offset and rotation
        point = [vertex[i] + offset[i] for i in range(3)]
        point = rotate_point(point, rotation[0], rotation[1], rotation[2])
        
        # Project to 2D
        projected = project_to_2d(point, width, height)
        if projected:
            projected_points.append(projected)
        else:
            projected_points.append(None)
    
    # Draw faces as connected points
    for face in faces:
        face_points = []
        for vertex_idx in face:
            if vertex_idx < len(projected_points) and projected_points[vertex_idx]:
                face_points.append(projected_points[vertex_idx])
        
        # Draw edges
        for i in range(len(face_points)):
            p1 = face_points[i]
            p2 = face_points[(i + 1) % len(face_points)]
            
            if p1 and p2:
                # Simple line drawing
                x1, y1 = p1
                x2, y2 = p2
                
                steps = max(abs(x2 - x1), abs(y2 - y1))
                if steps > 0:
                    for step in range(steps + 1):
                        t = step / steps
                        x = int(x1 + t * (x2 - x1))
                        y = int(y1 + t * (y2 - y1))
                        
                        if 0 <= x < width and 0 <= y < height:
                            screen[y][x] = char
    
    return screen

def combine_screens(screens):
    """Combine multiple ASCII screens into one"""
    if not screens:
        return []
    
    height = len(screens[0])
    width = sum(len(screen[0]) for screen in screens)
    
    combined = [[' ' for _ in range(width)] for _ in range(height)]
    
    x_offset = 0
    for screen in screens:
        for y in range(height):
            for x in range(len(screen[0])):
                if screen[y][x] != ' ':
                    combined[y][x + x_offset] = screen[y][x]
        x_offset += len(screen[0])
    
    return combined

def main():
    print("Interstate3D Vehicle with Weapons - ASCII 3D Preview")
    print("=" * 80)
    print()
    
    # Load models
    car_vertices, car_faces = load_obj_vertices('assets/models/player_car.obj')
    mg_vertices, mg_faces = load_obj_vertices('assets/models/machine_gun.obj')
    rl_vertices, rl_faces = load_obj_vertices('assets/models/rocket_launcher.obj')
    
    if not car_vertices:
        print("Could not load vehicle model")
        return
    
    print("Vehicle Model Loaded:")
    print(f"  Vertices: {len(car_vertices)}")
    print(f"  Faces: {len(car_faces)}")
    print()
    
    # Render different views
    views = []
    
    # Front view
    front_car = render_ascii_3d(car_vertices, car_faces, [0, 0, 0], [0, 0, 0], '#')
    front_mg = render_ascii_3d(mg_vertices, mg_faces, [0, 0.6, 1.0], [0, 0, 0], '*')
    front_rl = render_ascii_3d(rl_vertices, rl_faces, [0.5, 0.6, 0.8], [0, 0, 0], '+')
    
    if front_car and front_mg and front_rl:
        front_view = combine_screens([front_car, front_mg, front_rl])
        views.append(("Front View", front_view))
    
    # Side view
    side_car = render_ascii_3d(car_vertices, car_faces, [0, 0, 0], [0, math.pi/2, 0], '#')
    side_mg = render_ascii_3d(mg_vertices, mg_faces, [0, 0.6, 1.0], [0, math.pi/2, 0], '*')
    side_rl = render_ascii_3d(rl_vertices, rl_faces, [0.5, 0.6, 0.8], [0, math.pi/2, 0], '+')
    
    if side_car and side_mg and side_rl:
        side_view = combine_screens([side_car, side_mg, side_rl])
        views.append(("Side View", side_view))
    
    # Top view
    top_car = render_ascii_3d(car_vertices, car_faces, [0, 0, 0], [math.pi/2, 0, 0], '#')
    top_mg = render_ascii_3d(mg_vertices, mg_faces, [0, 0.6, 1.0], [math.pi/2, 0, 0], '*')
    top_rl = render_ascii_3d(rl_vertices, rl_faces, [0.5, 0.6, 0.8], [math.pi/2, 0, 0], '+')
    
    if top_car and top_mg and top_rl:
        top_view = combine_screens([top_car, top_mg, top_rl])
        views.append(("Top View", top_view))
    
    # Display views
    for view_name, screen in views:
        print(f"{view_name}:")
        print(f"  # = Vehicle Body (Blue)")
        print(f"  * = Machine Gun (Gray)")
        print(f"  + = Rocket Launcher (Green)")
        print()
        
        for row in screen:
            print(''.join(row))
        print()
    
    print("Legend:")
    print("  # - Blue player vehicle body with racing stripes")
    print("  * - Gray machine gun mounted on hood")
    print("  + - Green rocket launcher mounted on side")
    print()
    print("This represents the vehicle with weapons we created for Interstate3D!")

if __name__ == "__main__":
    main()
