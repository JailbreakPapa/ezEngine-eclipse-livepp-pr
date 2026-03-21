"""Core hair export logic: reads Blender hair curves and writes to Alembic."""

import bpy
import os
import tempfile

from .coordinate_utils import blender_to_ez


def get_hair_curve_objects(context, selected_only=True):
    """Find all hair curve objects in the scene or selection.

    Args:
        context: Blender context.
        selected_only: If True, only return selected objects.

    Returns:
        List of objects with hair curve data.
    """
    result = []
    objects = context.selected_objects if selected_only else context.scene.objects

    for obj in objects:
        if obj.type == 'CURVES':
            result.append(obj)
        elif obj.type == 'EMPTY' and hasattr(obj, 'particle_systems'):
            # Legacy particle hair - skip (we only support new curves)
            pass

    return result


def export_hair_curves_alembic(context, filepath, settings):
    """Export hair curves to an Alembic file using Blender's built-in exporter.

    Uses Blender's bpy.ops.wm.alembic_export with appropriate settings
    for hair curve data.

    Args:
        context: Blender context.
        filepath: Output .abc file path.
        settings: Dictionary of export settings.

    Returns:
        Set of {'FINISHED'} on success, {'CANCELLED'} on failure.
    """
    hair_objects = get_hair_curve_objects(context, settings.get('selected_only', True))

    if not hair_objects:
        return {'CANCELLED'}, "No hair curve objects found."

    # Ensure only hair curve objects are selected for export
    bpy.ops.object.select_all(action='DESELECT')
    for obj in hair_objects:
        obj.select_set(True)

    # Set the active object
    context.view_layer.objects.active = hair_objects[0]

    # Export using Blender's Alembic exporter
    try:
        bpy.ops.wm.alembic_export(
            filepath=filepath,
            selected=True,
            export_hair=True,
            export_particles=False,
            # Only export curves, not meshes
            visible_objects_only=False,
            flatten=False,
            # Frame range: just current frame for static hair
            start=context.scene.frame_current,
            end=context.scene.frame_current,
            # Geometry settings
            uvs=True,
            normals=True,
            face_sets=False,
            # Transform
            global_scale=settings.get('global_scale', 1.0),
        )
    except Exception as e:
        return {'CANCELLED'}, f"Alembic export failed: {str(e)}"

    return {'FINISHED'}, f"Exported {len(hair_objects)} hair curve object(s) to {filepath}"


def export_hair_manual(context, filepath, settings):
    """Manual hair export that reads curve data directly and writes via Python Alembic bindings.

    This is the fallback path when more control is needed than Blender's built-in
    exporter provides. Requires the 'alembic' Python package.

    Args:
        context: Blender context.
        filepath: Output .abc file path.
        settings: Dictionary of export settings.

    Returns:
        Tuple of (status_set, message_string).
    """
    try:
        import alembic
        from alembic import Abc, AbcGeom
    except ImportError:
        # Fall back to Blender's built-in exporter
        return export_hair_curves_alembic(context, filepath, settings)

    hair_objects = get_hair_curve_objects(context, settings.get('selected_only', True))
    if not hair_objects:
        return {'CANCELLED'}, "No hair curve objects found."

    global_scale = settings.get('global_scale', 1.0)
    width_scale = settings.get('width_scale', 1.0)

    # Create Alembic archive
    archive = Abc.OArchive(filepath)
    top = archive.getTop()

    total_strands = 0

    for obj in hair_objects:
        curves_data = obj.data  # bpy.types.Curves

        if not hasattr(curves_data, 'curves') or len(curves_data.curves) == 0:
            continue

        # Create an OCurves object for this hair object
        curves_obj = AbcGeom.OCurves(top, obj.name)
        curves_schema = curves_obj.getSchema()

        # Collect all curve data
        positions = []
        num_vertices = []
        widths = []

        world_matrix = obj.matrix_world

        for curve_idx in range(len(curves_data.curves)):
            curve = curves_data.curves[curve_idx]
            point_start = curve.points[0].index if hasattr(curve.points[0], 'index') else 0
            point_count = len(curve.points)
            num_vertices.append(point_count)

            for pt_idx in range(point_count):
                pt = curves_data.points[point_start + pt_idx]
                # Transform to world space
                world_pos = world_matrix @ pt.position
                # Convert to ezEngine coordinate system
                ez_pos = blender_to_ez(world_pos * global_scale)
                positions.extend(ez_pos)

                # Width/radius
                radius = pt.radius if hasattr(pt, 'radius') else 0.005
                widths.append(radius * width_scale * global_scale)

        # Write the sample
        sample = AbcGeom.OCurvesSchemaSample()
        sample.setPositions(positions)
        sample.setCurvesNumVertices(num_vertices)
        sample.setType(AbcGeom.kCubic)
        sample.setWrap(AbcGeom.kNonPeriodic)

        # Set widths
        width_sample = AbcGeom.OFloatGeomParamSample()
        width_sample.setVals(widths)
        width_sample.setScope(AbcGeom.kVertexScope)
        sample.setWidths(width_sample)

        curves_schema.set(sample)
        total_strands += len(curves_data.curves)

    return {'FINISHED'}, f"Exported {total_strands} strands from {len(hair_objects)} object(s) to {filepath}"
