bl_info = {
    "name": "ezEngine Hair Exporter",
    "author": "ezEngine Contributors",
    "version": (1, 0, 0),
    "blender": (3, 3, 0),
    "location": "View3D > Sidebar > ezEngine",
    "description": "Export Blender Hair Curves to Alembic (.abc) for ezEngine",
    "category": "Import-Export",
}

import bpy
from bpy.props import StringProperty, FloatProperty, BoolProperty, IntProperty, EnumProperty

from . import operators
from . import panels


class EzHairExporterPreferences(bpy.types.AddonPreferences):
    bl_idname = __name__

    default_export_path: StringProperty(
        name="Default Export Path",
        description="Default directory for exported .abc files",
        subtype='DIR_PATH',
        default="//",
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "default_export_path")


classes = (
    EzHairExporterPreferences,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    operators.register()
    panels.register()


def unregister():
    panels.unregister()
    operators.unregister()
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
