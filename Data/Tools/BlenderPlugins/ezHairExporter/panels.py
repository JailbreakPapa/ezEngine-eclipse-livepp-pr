"""UI panels for ezEngine hair export in Blender's sidebar."""

import bpy

from .hair_export import get_hair_curve_objects


class EZHAIR_PT_main_panel(bpy.types.Panel):
    """ezEngine Hair Export panel in the 3D viewport sidebar."""
    bl_label = "ezEngine Hair Export"
    bl_idname = "EZHAIR_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "ezEngine"

    def draw(self, context):
        layout = self.layout

        # Show selected hair curve objects
        hair_objects = get_hair_curve_objects(context, selected_only=True)

        if hair_objects:
            box = layout.box()
            box.label(text=f"Selected Hair Objects: {len(hair_objects)}", icon='OUTLINER_OB_CURVES')

            total_strands = 0
            for obj in hair_objects:
                if hasattr(obj.data, 'curves'):
                    strand_count = len(obj.data.curves)
                    total_strands += strand_count
                    row = box.row()
                    row.label(text=f"  {obj.name}: {strand_count} strands")

            box.label(text=f"Total: {total_strands} strands")
        else:
            layout.label(text="No hair curves selected", icon='INFO')
            layout.label(text="Select Hair Curve objects to export")

        layout.separator()

        # Export buttons
        col = layout.column(align=True)
        col.scale_y = 1.5
        col.operator("ezhair.export", text="Export Hair...", icon='EXPORT')
        col.operator("ezhair.quick_export", text="Quick Export", icon='FILE_TICK')

        if not hair_objects:
            col.enabled = False


class EZHAIR_PT_info_panel(bpy.types.Panel):
    """Information panel about the selected hair curves."""
    bl_label = "Hair Info"
    bl_idname = "EZHAIR_PT_info_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "ezEngine"
    bl_parent_id = "EZHAIR_PT_main_panel"
    bl_options = {'DEFAULT_CLOSED'}

    def draw(self, context):
        layout = self.layout

        hair_objects = get_hair_curve_objects(context, selected_only=True)

        if not hair_objects:
            layout.label(text="No hair curves selected")
            return

        for obj in hair_objects:
            box = layout.box()
            box.label(text=obj.name, icon='OUTLINER_OB_CURVES')

            if hasattr(obj.data, 'curves') and len(obj.data.curves) > 0:
                curves = obj.data.curves
                box.label(text=f"Strands: {len(curves)}")
                box.label(text=f"Total Points: {len(obj.data.points)}")

                # Calculate average points per strand
                if len(curves) > 0:
                    total_points = len(obj.data.points)
                    avg_points = total_points / len(curves)
                    box.label(text=f"Avg Points/Strand: {avg_points:.1f}")

                # Show bounding box info
                if obj.bound_box:
                    bb = obj.bound_box
                    min_x = min(v[0] for v in bb)
                    max_x = max(v[0] for v in bb)
                    min_y = min(v[1] for v in bb)
                    max_y = max(v[1] for v in bb)
                    min_z = min(v[2] for v in bb)
                    max_z = max(v[2] for v in bb)
                    size_x = max_x - min_x
                    size_y = max_y - min_y
                    size_z = max_z - min_z
                    box.label(text=f"Size: {size_x:.3f} x {size_y:.3f} x {size_z:.3f}m")


classes = (
    EZHAIR_PT_main_panel,
    EZHAIR_PT_info_panel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
