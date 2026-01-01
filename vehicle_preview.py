#!/usr/bin/env python3
"""
Simple vehicle and weapon preview using matplotlib
Visualizes the 3D models we created for Interstate3D
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

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

def create_vehicle_preview():
    """Create a 3D visualization of vehicle with weapons"""
    
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    # Load player car
    car_vertices, car_faces = load_obj_vertices('assets/models/player_car.obj')
    
    # Load machine gun
    mg_vertices, mg_faces = load_obj_vertices('assets/models/machine_gun.obj')
    
    # Load rocket launcher  
    rl_vertices, rl_faces = load_obj_vertices('assets/models/rocket_launcher.obj')
    
    if car_vertices and car_faces:
        # Draw car body (blue)
        car_mesh = [[car_vertices[idx] for idx in face] for face in car_faces]
        car_collection = Poly3DCollection(car_mesh, alpha=0.7, facecolor='blue', edgecolor='darkblue')
        ax.add_collection3d(car_collection)
    
    if mg_vertices and mg_faces:
        # Draw machine gun (gray) - position on car
        mg_mesh = [[mg_vertices[idx] for idx in face] for face in mg_faces]
        # Offset position to mount on car
        mg_mesh_offset = [[[v[0], v[1]+0.6, v[2]+1.0] for v in face] for face in mg_mesh]
        mg_collection = Poly3DCollection(mg_mesh_offset, alpha=0.8, facecolor='gray', edgecolor='darkgray')
        ax.add_collection3d(mg_collection)
    
    if rl_vertices and rl_faces:
        # Draw rocket launcher (green) - position on car
        rl_mesh = [[rl_vertices[idx] for idx in face] for face in rl_faces]
        # Offset position to mount on car
        rl_mesh_offset = [[[v[0]+0.5, v[1]+0.6, v[2]+0.8] for v in face] for face in rl_mesh]
        rl_collection = Poly3DCollection(rl_mesh_offset, alpha=0.8, facecolor='darkgreen', edgecolor='black')
        ax.add_collection3d(rl_collection)
    
    # Set labels and title
    ax.set_xlabel('X')
    ax.set_ylabel('Y') 
    ax.set_zlabel('Z')
    ax.set_title('Interstate3D Vehicle with Weapons Preview\nBlue Car + Gray Machine Gun + Green Rocket Launcher')
    
    # Set equal aspect ratio and limits
    max_range = 3
    ax.set_xlim([-max_range, max_range])
    ax.set_ylim([-max_range, max_range])
    ax.set_zlim([-max_range, max_range])
    
    # Set viewing angle
    ax.view_init(elev=20, azim=45)
    
    plt.tight_layout()
    plt.savefig('/home/u1/CascadeProjects/interstate3d/vehicle_with_weapons.png', dpi=150, bbox_inches='tight')
    plt.show()

if __name__ == "__main__":
    create_vehicle_preview()
