"""Coordinate system conversion between Blender and ezEngine."""


def blender_to_ez(pos):
    """Convert a position from Blender space (Z-up, right-handed) to ezEngine space (Y-up, left-handed).

    Blender: X-right, Y-forward, Z-up, right-handed, 1 unit = 1 meter
    ezEngine: X-right, Y-up, Z-forward, left-handed

    Args:
        pos: A tuple/Vector of (x, y, z) in Blender space.

    Returns:
        Tuple (x, z, -y) in ezEngine space.
    """
    return (pos[0], pos[2], -pos[1])


def blender_to_ez_vec(vec):
    """Convert a direction vector from Blender space to ezEngine space.

    Same axis swap as position, but without translation concerns.
    """
    return (vec[0], vec[2], -vec[1])


def blender_to_ez_scale(scale):
    """Convert a scale vector from Blender space to ezEngine space."""
    return (scale[0], scale[2], scale[1])
